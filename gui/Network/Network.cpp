#include "Network.hpp"
#include "logging/Logger.hpp"

#include <netinet/in.h>

#include <sstream>
#include <stdexcept>

constexpr int FD_PIPE_EXIT = 0;
constexpr int FD_SERVER_IN = 1;
constexpr int FD_PIPE_NETWORK = 2;
constexpr int FD_EXIT_IN = 0;
constexpr int FD_EXIT_OUT = 1;
constexpr int FD_SERVER_OUT = 0;

Network::Network(int port, std::string hostname, std::shared_ptr<API> &data)
  : _port(port), _hostname(std::move(hostname)), _api(data)
{
  // Create server socket
  _fdServer = socket(AF_INET, SOCK_STREAM, 0);
  if (_fdServer == -1)
    throw std::
      runtime_error("Error: socket, Function: Network, File: Network.cpp");

  // Set socket options
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_port);
  socklen_t sockLen = sizeof(serverAddr);
  // Convert hostname to IP address
  if (inet_pton(AF_INET, _hostname.c_str(), &serverAddr.sin_addr) != 1)
    throw std::runtime_error(
      "Error: inet_pton failed. Function: RunNetwork, File: Network.cpp");

  // Try to connect to the server in a loop until successful or unrecoverable
  // error
  while (true) {
    if (connect(_fdServer, (const sockaddr *)(&serverAddr), sockLen) == 0)
      break;  // Connected successfully
    // Check for recoverable errors
    if (errno == EINTR || errno == ECONNREFUSED || errno == ETIMEDOUT) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      continue;
    }
    // Unrecoverable error
    throw std::runtime_error(
      std::string(
        "Error: connect failed. Function: Network, File: "
        "Network.cpp: (")
      + strerror(errno) + ").");
  }

  // Create pipe for thread exit
  if (pipe(_pipeFdExit.data()) == -1)
    throw std::runtime_error(
      "Error: pipe failed. Function: Network, File: Network.cpp");

  // Initialize poll structures
  _pollInFd[FD_PIPE_EXIT].fd = _pipeFdExit[FD_EXIT_IN];
  _pollInFd[FD_SERVER_IN].fd = _fdServer;
  _pollInFd[FD_PIPE_NETWORK].fd = _api->GetInFd();
  for (pollfd &pollFd: _pollInFd) {
    pollFd.events = POLLIN | POLLHUP;  // monitor for input and hangup
    pollFd.revents = 0;
  }

  // Initialize pollOutFd for sending messages
  _pollOutFd[FD_SERVER_OUT].events = POLLOUT;
  _pollOutFd[FD_SERVER_OUT].fd = _fdServer;
  _pollOutFd[FD_SERVER_OUT].revents = 0;
}

Network::~Network()
{
  _networkThread.join();  // wait for the network thread to finish

  shutdown(_fdServer, SHUT_RDWR);  // shut both end read and write
  read(_fdServer, nullptr, 0);     // clear buffer
  close(_fdServer);                // close socket

  close(_pipeFdExit[FD_EXIT_IN]);   // close the read end of the pipe
  close(_pipeFdExit[FD_EXIT_OUT]);  // close the write end of the pipe
}

void Network::ServerHandshake()
{
  if (poll(_pollInFd.data(), _pollInFd.size(), -1) == -1)
    throw std::runtime_error(
      "Error: poll failed. Function: RunNetwork, File: Network.cpp");

  if (_pollInFd[FD_SERVER_IN].revents & POLLIN)
    Log::inf << "Message received: " << Log::cleanString(ReceiveMessage());

  SendMessage("GRAPHIC\n");
}

void Network::Run()
{
  if (!_networkThread.joinable())
    _networkThread = std::thread(&Network::RunNetworkInternal, this);
}

void Network::RunNetworkInternal()
{
  ServerHandshake();

  bool isRunning = true;
  while (isRunning) {
    if (poll(_pollInFd.data(), _pollInFd.size(), -1) == -1)
      throw std::runtime_error(
        "Error: poll failed. Function: RunNetwork, File: Network.cpp");

    // server disconnected
    if (_pollInFd[FD_SERVER_IN].revents & POLLHUP) {
      Log::failed << "Server disconnected.";
      isRunning = false;
      continue;
    }

    // exit event
    if (_pollInFd[FD_PIPE_EXIT].revents & POLLIN) {
      std::array<char, 10> buffer;
      if (read(_pollInFd[FD_PIPE_EXIT].fd, buffer.data(), buffer.size()) == -1)
        throw std::runtime_error(
          "Error: read failed. Function: RunNetwork, File: Network.cpp");
      isRunning = false;
      Log::inf << "End of network thread requested.";
    }

    // server message
    if (_pollInFd[FD_SERVER_IN].revents & POLLIN) {
      std::string message = ReceiveMessage();
      try {
        _api->ParseManageCommand(message);
      } catch (const std::exception &e) {
        Log::failed << "Exception in ParseManageCommande: " << e.what();
      }
    }

    // network pipe message
    if (_pollInFd[FD_PIPE_NETWORK].revents & POLLIN) {
      static std::string leftover;
      std::string message;
      std::array<char, 1024> buffer;

      ssize_t bytesRead = read(
        _api->GetInFd(), buffer.data(), buffer.size() - 1);
      if (bytesRead < 0)
        throw std::runtime_error(
          "Error: read failed. Function: RunNetwork, File: Network.cpp");
      // all messages are separated by '\n'
      buffer[bytesRead] = '\0';
      std::stringstream ss(leftover + std::string(buffer.data(), bytesRead));
      while (std::getline(ss, message, '\n'))
        if (!message.empty()) {
          // Log::inf
          //   << "Message received from pipe: " << Log::cleanString(message);
          SendMessage(message + "\n");
        }
      // if there is still data in the stream, save it for the next iteration
      // (e.g., if the last message did not end with '\n')
      leftover.clear();
      if (!ss.eof())
        leftover = ss.str().substr(ss.tellg());
    }
  }
}

void Network::SendMessage(const std::string &msg)
{
  if (poll(_pollOutFd.data(), _pollOutFd.size(), -1) == -1)
    throw std::runtime_error(
      "Error: poll failed, Function: SendMessage, File: Network.cpp");

  // Check if the socket is ready for writing
  if (!(_pollOutFd[FD_SERVER_OUT].revents & POLLOUT))
    throw std::runtime_error(
      "Error: socket not ready for writing, Function: SendMessage, File: "
      "Network.cpp");

  if (send(_fdServer, msg.c_str(), msg.size(), 0) == -1)
    throw std::
      runtime_error("Error: send, Function: SendMessage, File: Network.cpp");
  // Log::inf << "Message sent : " << Log::cleanString(msg);
}

std::string Network::ReceiveMessage() const
{
  constexpr std::size_t chunk_size = 1024;
  std::vector<char> buffer;

  while (true) {
    std::vector<char> chunk(chunk_size);
    ssize_t bytes_received = recv(_fdServer, chunk.data(), chunk.size(), 0);
    if (bytes_received < 0) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error(
        "Error: recv failed, Function: ReceiveMessage, File: Network.cpp");
    }
    if (bytes_received == 0)
      break;
    buffer.insert(buffer.end(), chunk.begin(), chunk.begin() + bytes_received);
    if ((size_t)(bytes_received) < chunk_size)
      break;
  }

  return std::string(buffer.begin(), buffer.end());
}

void Network::RequestStop()
{
  Log::inf << "Requesting stop of network thread.\n";
  write(_pipeFdExit[FD_EXIT_OUT], "x", 1);
}

#include "Network.hpp"
#include "logging/Logger.hpp"

#include <arpa/inet.h>
#include <array>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

Network::Network(int port, const std::string &hostname)
  : _port(port), _hostname(hostname)
{
  _fdServer = socket(AF_INET, SOCK_STREAM, 0);
  if (_fdServer < 0) {
    throw std::
      runtime_error("Error: socket, Function: Network, File: Network.cpp");
  }
}

void Network::RunNetwork()
{
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_port);
  socklen_t sockLen = sizeof(serverAddr);

  if (inet_pton(AF_INET, _hostname.c_str(), &serverAddr.sin_addr) != 1)
    throw std::runtime_error(
      "Error: inet_pton failed. Function: RunNetwork, File: Network.cpp");

  if (
    connect(
      _fdServer, reinterpret_cast<const sockaddr *>(&serverAddr), sockLen)
    < 0) {
    throw std::runtime_error(
      "Error: connect failed. Function: RunNetwork, File: Network.cpp");
  }

  bool end = false;
  std::array<struct pollfd, 1> pollFd;
  pollFd[0].fd = _fdServer;
  pollFd[0].events = POLLIN | POLLOUT;
  pollFd[0].revents = 0;

  if (poll(pollFd.data(), 1, -1) == -1)
    throw std::runtime_error(
      "Error: poll failed. Function: RunNetwork, File: Network.cpp");

  if (pollFd[0].revents & POLLIN)
    Log::info << "Message received: " << Log::cleanString(ReceiveMessage());

  if (pollFd[0].revents & POLLOUT) {
    std::string msg = "GRAPHIC\n";
    SendMessage(msg);
  }

  while (!end) {
    if (poll(pollFd.data(), 1, -1) == -1)
      throw std::runtime_error(
        "Error: poll failed. Function: RunNetwork, File: Network.cpp");

    if (pollFd[0].revents & POLLIN) {
      std::string message = ReceiveMessage();
      api.ParseManageCommande(message);
    }
    if (pollFd[0].revents & POLLOUT) {
      for (std::string command: api.GetCommand())
        SendMessage(command);
      api.ClearCommand();
    }
  }
}

void Network::SendMessage(std::string &msg) const
{
  Log::info << "Message sent : " << Log::cleanString(msg);
  if (send(_fdServer, msg.c_str(), msg.size(), 0) == -1)
    throw std::
      runtime_error("Error: send, Function: SendMessage, File: Network.cpp");
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
    if (static_cast<std::size_t>(bytes_received) < chunk_size)
      break;
  }

  return std::string(buffer.begin(), buffer.end());
}

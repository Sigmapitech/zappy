#include "Network.hpp"
#include "logging/Logger.hpp"

#include <arpa/inet.h>
#include <array>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
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
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(_port);

  if (inet_pton(AF_INET, _hostname.c_str(), &server_addr.sin_addr) != 1) {
    throw std::runtime_error(
      "Error: inet_pton failed. Function: RunNetwork, File: Network.cpp");
  }

  if (
    connect(
      _fdServer,
      reinterpret_cast<const sockaddr *>(&server_addr),
      sizeof(server_addr))
    < 0) {
    throw std::runtime_error(
      "Error: connect failed. Function: RunNetwork, File: Network.cpp");
  }

  bool end = false;
  auto poll_fd = CreatePollFd(_fdServer, POLLIN | POLLOUT);

  if (poll(poll_fd.data(), 1, -1) == -1) {
    throw std::runtime_error(
      "Error: poll failed. Function: RunNetwork, File: Network.cpp");
  }

  if ((poll_fd[0].revents & POLLIN) != 0)
    Log::info << "Message received: " << Log::cleanString(ReceiveMessage());

  if ((poll_fd[0].revents & POLLOUT) != 0) {
    std::string msg = "GRAPHIC\n";
    SendMessage(msg);
  }

  while (!end) {
    if (poll(poll_fd.data(), 1, -1) == -1) {
      throw std::runtime_error(
        "Error: poll failed. Function: RunNetwork, File: Network.cpp");
    }

    if (poll_fd[0].revents & POLLIN) {
      Log::info << "Message received: " << Log::cleanString(ReceiveMessage());
      // add ParserMessage
    }
  }
}

void Network::SendMessage(std::string &msg) const
{
  Log::info << "Message sent : " << Log::cleanString(msg);
  if (send(_fdServer, msg.c_str(), msg.size(), 0) == -1) {
    throw std::
      runtime_error("Error: send, Function: SendMessage, File: Network.cpp");
  }
}

std::string Network::ReceiveMessage() const
{
  constexpr std::size_t buffer_size = 65536;
  std::vector<char> buffer(buffer_size);

  ssize_t bytes_received = recv(_fdServer, buffer.data(), buffer_size, 0);

  if (bytes_received < 0) {
    throw std::runtime_error(
      "Error: recv failed, Function: ReceiveMessage, File: Network.cpp");
  }
  if (bytes_received == 0)
    return {};
  return std::string(buffer.data(), static_cast<std::size_t>(bytes_received));
}

std::array<struct pollfd, 1> Network::CreatePollFd(int fd, short events)
{
  std::array<struct pollfd, 1> pollList;
  pollList[0].fd = fd;
  pollList[0].events = events;
  pollList[0].revents = 0;
  return pollList;
}

#include "Network.hpp"
#include "../logging/Logger.hpp"

Network::Network(uint16_t port, const std::string &hostname)
  : _port(port), _hostname(std::move(hostname))
{
  _fdServer = socket(AF_INET, SOCK_STREAM, 0);
  if (_fdServer < 0) {
    throw(std::runtime_error(
      "Error: socket, Function Network, File: "
      "Network.cpp"));
  }
}

void Network::RunNetwork()
{
  struct sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_port);

  if (inet_pton(AF_INET, _hostname.c_str(), &serverAddr.sin_addr) <= 0)
    throw(std::runtime_error(
      "Error: inet_pton, Function: RunNewtwork, File: "
      "Network.cpp"));

  if (
    connect(
      _fdServer,
      reinterpret_cast<struct sockaddr *>(&serverAddr),
      sizeof(serverAddr))
    < 0)
    throw(std::runtime_error(
      "Error: connect, Function: RunNewtwork, File: "
      "Network.cpp"));

  bool end = false;
  struct pollfd *poll_fd = CreatePollFd(_fdServer, POLLIN, 0);

  Network::PollMaker(poll_fd, 1, -1);
  Log::info << "Message receive : " << Log::cleanString(ReceiveMessage());
  std::string msg = "GRAPHIC\n";
  SendMessage(msg);

  while (!end) {
    Network::PollMaker(poll_fd, 1, -1);
    if (poll_fd[0].revents & POLLIN)
      Log::info << "Message receive : " << Log::cleanString(ReceiveMessage());
    // Parsing of messages.
  }
  delete (poll_fd);
}

void Network::SendMessage(std::string &msg) const
{
  Log::info << "Message sent : " << Log::cleanString(msg);
  if (send(_fdServer, msg.c_str(), msg.size(), 0) == -1)
    throw(std::runtime_error(
      "Error: send, Function: SendMessage, File: "
      "Network.cpp"));
}

std::string Network::ReceiveMessage() const
{
  std::vector<char> buffer(MAX_MESSAGE_SIZE);

  ssize_t bytesReceived = recv(_fdServer, buffer.data(), buffer.size() - 1, 0);
  if (bytesReceived <= 0) {
    throw(std::runtime_error(
      "Error: recv, Function: ReceiveMessage, File: "
      "Network.cpp"));
  }
  buffer[bytesReceived] = '\0';
  std::string message(buffer.data());

  return message;
}

struct pollfd *Network::CreatePollFd(int fd, short event, short revent)
{
  auto *pollList = new struct pollfd[1];

  pollList[0].fd = fd;
  pollList[0].events = event;
  pollList[0].revents = revent;
  return pollList;
}

void Network::PollMaker(struct pollfd *pollList, int sizePollList, int timeout)
{
  int ret = poll(pollList, sizePollList, timeout);
  if (ret < 0)
    throw(std::runtime_error(
      "Error: poll, Function: RunNetwork, File: "
      "Network.cpp"));
}

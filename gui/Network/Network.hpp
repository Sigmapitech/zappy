#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

class Network {
private:
  int _port;
  const std::string _hostname;
  int _fdServer;

public:
  /**
   * @brief Construct a new Network object.
   *
   * @param port Contain the port of the server.
   * @param hostname Contain the hostname of the server.
   */
  Network(int port, const std::string &hostname);
  ~Network() = default;

  /**
   * @brief Run the network of the client.
   */
  void RunNetwork();

  /**
   * @brief Send a message to the server.
   *
   * @param msg Contain the message to send.
   */
  void SendMessage(std::string &msg) const;

  /**
   * @brief Receive a message from the server.
   *
   * @return Return as a std::string the message sent by the server.
   */
  [[nodiscard]] std::string ReceiveMessage() const;

  /**
   * @brief Create a Poll Fd object.
   *
   * @param fd Contain the file director the poll need to check.
   * @param event Contain the event the poll need to check on the fd.
   * @param revent Contain the revent that will allow to interact.
   * @return Return as a struct pollfd* the pollfd needed by poll
   */
  static std::array<struct pollfd, 1> CreatePollFd(int fd, short events);
};

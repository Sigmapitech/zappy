#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define MAX_MESSAGE_SIZE 1024

class Network {
private:
  int _port;
  const std::string &_hostname;
  int _fdServer;

public:
  /**
   * @brief Construct a new Network object.
   *
   * @param port Contain the port of the server.
   * @param hostname Contain the hostname of the server.
   */
  Network(uint16_t port, const std::string &hostname);
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
  static struct pollfd *CreatePollFd(int fd, short event, short revent);

  /**
   * @brief Use poll
   *
   * @param pollList Contain the list of pollfd checked by poll.
   * @param sizePollList Contain the number of element in the list.
   * @param timeout Contain the time when the poll need to check.
   */
  static void
  PollMaker(struct pollfd *pollList, int sizePollList, int timeout);
};

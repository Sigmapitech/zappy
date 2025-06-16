#pragma once

#include "API/API.hpp"

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
  API api;

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
};

#pragma once

#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "API/API.hpp"

#include <array>
#include <memory>
#include <string>
#include <thread>

class Network {
private:
  int _port;
  const std::string _hostname;
  sockaddr_in serverAddr;
  int _fdServer;
  std::shared_ptr<API> _api = nullptr;
  std::thread _networkThread;
  std::array<int, 2> _pipeFdExit;
  std::array<pollfd, 3> _pollInFd;
  std::array<pollfd, 1> _pollOutFd;

  /**
   * @brief Run the network of the client.
   */
  void RunNetworkInternal();

  /**
   * @brief Perform the client handshake with the server.
   */
  void ServerHandshake();

public:
  /**
   * @brief Construct a new Network object.
   *
   * @param port Contain the port of the server.
   * @param hostname Contain the hostname of the server.
   */
  Network(int port, std::string hostname, std::shared_ptr<API> &data);
  ~Network();

  /**
   * @brief Run the network of the client.
   */
  void Run();

  /**
   * @brief Send a message to the server.
   *
   * @param msg Contain the message to send.
   */
  void SendMessage(const std::string &msg);

  /**
   * @brief Receive a message from the server.
   *
   * @return Return as a std::string the message sent by the server.
   */
  [[nodiscard]] std::string ReceiveMessage() const;

  /**
   * @brief Request to stop the network thread
   */
  void RequestStop();
};

#ifndef SOCKET_H
#define SOCKET_H

#include <string>

class Socket {
public:
  Socket(int type, const std::string &address, int port);
  bool connect();
  void sendPair(const std::pair<int, std::string> &data);
  std::pair<int, std::string> receivePair();
  void close();

private:
  int socketType;
  std::string serverAddress;
  int serverPort;
  int socketDescriptor;
  // std::mutex mutex;
  // std::condition_variable cv;
  // bool dataSent;
};

#endif
#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <arpa/inet.h>
#include <queue>
#include <utility>
class Socket {
public:
  int socketDescriptor;
  int socketType;
  std::string serverAddress;
  struct sockaddr_in serverAddressInfo;
  int serverPort;

  Socket(int type, const std::string &address, int port);
  Socket(int data);
  int getSocketDescriptor();
  void close();
  void Connect();
  void beClient();
  std::pair<std::string, std::string> Deserialize(const char *input);
  char *Serialize(const std::pair<std::string, std::string> &input);
  void Serve(std::queue<std::pair<std::string, std::string>> &dataQueue);
};

#endif

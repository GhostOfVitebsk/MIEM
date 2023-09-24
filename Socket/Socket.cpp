#include "Socket.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <utility>

Socket::Socket(int type, const std::string &address, int port)
    : socketType(type), serverAddress(address), serverPort(port) {
  if (socketType == 1) {
    socketDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
  } else if (socketType == 2) {
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  } else {
    std::cerr << "Invalid socket type!" << std::endl;
    socketDescriptor = -1;
  }
}
bool Socket::connect() {
  if (socketDescriptor == -1) {
    std::cerr << "Socket is not initialized!" << std::endl;
    return false;
  }

  if (socketType == 1) {
    struct sockaddr_un serverAddress {};
    serverAddress.sun_family = AF_UNIX;
    strncpy(serverAddress.sun_path, serverAddress.sun_path,
            sizeof(serverAddress.sun_path) - 1);

    if (::connect(socketDescriptor, (struct sockaddr *)&serverAddress,
                  sizeof(serverAddress)) == -1) {
      std::cerr << "Failed to connect to Unix socket!" << std::endl;
      return false;
    }
  } else if (socketType == 2) {
    struct sockaddr_in ServerAddress {};
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverAddress.c_str(), &(ServerAddress.sin_addr));

    if (::connect(socketDescriptor, (struct sockaddr *)&ServerAddress,
                  sizeof(ServerAddress)) == -1) {
      std::cerr << "Failed to connect to TCP socket!" << std::endl;
      return false;
    }
  } else {
    std::cerr << "Invalid socket type!" << std::endl;
    return false;
  }
  return true;
}

void Socket::sendPair(const std::pair<int, std::string> &data) {
  // std::unique_lock<std::mutex> lock(mutex);

  if (socketDescriptor == -1) {
    std::cerr << "Socket is not initialized!" << std::endl;
    return;
  }

  std::string pairString = std::to_string(data.first) + ":" + data.second;

  if (::send(socketDescriptor, pairString.c_str(), pairString.length(), 0) ==
      -1) {
    std::cerr << "Failed to send data over the socket!" << std::endl;
    return;
  }
  // dataSent = true;
  // cv.notify_all();
}

std::pair<int, std::string> Socket::receivePair() {
  // std::unique_lock<std::mutex> lock(mutex);

  if (socketDescriptor == -1) {
    std::cerr << "Socket is not initialized!" << std::endl;
    return std::make_pair(-1, "");
  }

  // cv.wait(lock, [this] { return dataSent; });

  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));

  ssize_t bytesRead = ::recv(socketDescriptor, buffer, sizeof(buffer) - 1, 0);
  if (bytesRead == -1) {
    std::cerr << "Failed to receive data from the socket!" << std::endl;
    return std::make_pair(-1, "");
  }

  std::string receivedData(buffer, bytesRead);
  size_t delimiterPos = receivedData.find(':');
  if (delimiterPos == std::string::npos) {
    std::cerr << "Received data is not in the expected format!" << std::endl;
    return std::make_pair(-1, "");
  }

  int intValue = std::stoi(receivedData.substr(0, delimiterPos));
  std::string stringValue = receivedData.substr(delimiterPos + 1);

  // dataSent = false;
  // cv.notify_all();

  return std::make_pair(intValue, stringValue);
}

void Socket::close() {
  if (socketDescriptor != -1) {
    ::close(socketDescriptor);
    socketDescriptor = -1;
  }
}

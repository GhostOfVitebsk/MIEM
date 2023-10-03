#include "Socket.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
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
    serverAddressInfo.sin_family = AF_INET;
    serverAddressInfo.sin_addr.s_addr = inet_addr(serverAddress.c_str());
    serverAddressInfo.sin_port = htons(serverPort);
  } else if (socketType == 3) {
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    serverAddressInfo.sin_family = AF_INET;
    serverAddressInfo.sin_addr.s_addr = inet_addr(serverAddress.c_str());
    serverAddressInfo.sin_port = htons(serverPort);
  } else {
    std::cerr << "Invalid socket type!" << std::endl;
    socketDescriptor = -1;
  }
}

Socket::Socket(int data) { socketDescriptor = data; }

int Socket::getSocketDescriptor() { return socketDescriptor; }
void Socket::close() {
  if (socketDescriptor != -1) {
    ::close(socketDescriptor);
    socketDescriptor = -1;
  }
}
std::pair<std::string, std::string> Socket::Deserialize(const char *input) {
  std::string str(input);
  size_t delimiterPos = str.find(':');
  if (delimiterPos == std::string::npos) {
    return std::make_pair("", "");
  }

  std::string first = str.substr(0, delimiterPos);
  std::string second = str.substr(delimiterPos + 1);

  return std::make_pair(first, second);
}
char *Socket::Serialize(const std::pair<std::string, std::string> &input) {
  std::string result = input.first + ":" + input.second;

  char *output = new char[result.length() + 1];
  strcpy(output, result.c_str());

  return output;
}
void Socket::Serve(std::queue<std::pair<std::string, std::string>> &dataQueue) {
  std::mutex mutex;
  std::condition_variable cv;

  std::thread receiverThread([&]() {
    char buffer[1024];

    while (true) {

      ssize_t bytesRead = recv(socketDescriptor, buffer, sizeof(buffer), 0);

      if (bytesRead <= 0) {

        break;
      }

      std::pair<std::string, std::string> deserializedData =
          Deserialize(buffer);
      std::cout << "Received: " << deserializedData.first << " "
                << deserializedData.second << std::endl;

      std::unique_lock<std::mutex> lock(mutex);
      dataQueue.push(deserializedData);
      lock.unlock();

      cv.notify_one();
    }
  });

  std::thread senderThread([&]() {
    while (true) {

      std::unique_lock<std::mutex> lock(mutex);

      cv.wait(lock, [&]() { return !dataQueue.empty(); });

      std::pair<std::string, std::string> data = dataQueue.front();
      dataQueue.pop();
      // Здесь будет полезная работа пока что заглушка
      std::cout << "Doing job " << data.first << " with data " << data.second << std::endl;
      lock.unlock();

      char *serializedData = Serialize(data);

      ssize_t bytesSent =
          send(socketDescriptor, serializedData, strlen(serializedData), 0);

      delete[] serializedData;

      if (bytesSent <= 0) {
        break;
      }
    }
  });
  receiverThread.join();
  senderThread.join();
}

void Socket::Connect() {
  if (connect(socketDescriptor, (struct sockaddr *)&serverAddressInfo,
              sizeof(serverAddressInfo)) < 0) {
    std::cout << "Failed to connect to the server" << std::endl;
    return;
  }
  std::cout << "Connected to the server" << std::endl;
}

void Socket::beClient() {

  std::mutex mutex;
  std::condition_variable cv;

  std::thread senderThread([&]() {
    while (true) {

      std::unique_lock<std::mutex> lock(mutex);

      std::string instruction, payload;
      std::cin >> instruction >> payload;
      std::pair<std::string, std::string> data =
          std::make_pair(instruction, payload);

      lock.unlock();

      char *serializedData = Serialize(data);
      printf("Sending: %s\n", serializedData);

      ssize_t bytesSent =
          send(socketDescriptor, serializedData, strlen(serializedData), 0);

      delete[] serializedData;

      if (bytesSent <= 0) {
        break;
      }
    }
  });
  std::thread receiverThread([&]() {
    char buffer[1024];

    while (true) {

      ssize_t bytesRead = recv(socketDescriptor, buffer, sizeof(buffer), 0);

      if (bytesRead <= 0) {

        break;
      }

      std::pair<std::string, std::string> deserializedData =
          Deserialize(buffer);
      std::cout << "Received: " << deserializedData.first << " "
                << deserializedData.second << std::endl;

      std::unique_lock<std::mutex> lock(mutex);

      cv.notify_one();
    }
  });
  receiverThread.join();
  senderThread.join();
}

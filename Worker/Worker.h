#ifndef WORKER_H
#define WORKER_H

#include <functional>
#include <iostream>
#include <sqlite3.h>
#include <string>

class Worker {
public:
  typedef std::function<void(const std::string &)> FunctionType;

  Worker(const std::string &dbFile);
  ~Worker();

  void registerFunction(int key, FunctionType function);
  void callFunction(int key, const std::string &signal);

private:
  std::string dbFile;
  sqlite3 *db;

  void createTable();
  std::string serializeFunction(const FunctionType &function);
  FunctionType deserializeFunction(const std::string &serializedFunction);
};

#endif
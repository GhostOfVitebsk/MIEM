#include "Worker.h"
#include <functional>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <string>

Worker::Worker(const std::string &dbFile) : dbFile(dbFile), db(nullptr) {
  if (sqlite3_open(dbFile.c_str(), &db) != SQLITE_OK) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
  } else {
    createTable();
  }
}

Worker::~Worker() {
  if (db) {
    sqlite3_close(db);
  }
}

void Worker::registerFunction(int key, FunctionType function) {
  std::string serializedFunction = serializeFunction(function);
  std::string query = "INSERT INTO functions (key, function) VALUES (?, ?);";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, key);
    sqlite3_bind_text(stmt, 2, serializedFunction.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to insert function: " << sqlite3_errmsg(db)
                << std::endl;
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db)
              << std::endl;
  }
}

void Worker::callFunction(int key, const std::string &signal) {
  std::string query = "SELECT function FROM functions WHERE key = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, key);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char *functionBody = sqlite3_column_text(stmt, 0);
      std::string serializedFunction(
          reinterpret_cast<const char *>(functionBody));
      FunctionType function = deserializeFunction(serializedFunction);
      function(signal);
    } else {
      std::cerr << "Function notfound" << std::endl;
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db)
              << std::endl;
  }
}

void Worker::createTable() {
  std::string query = "CREATE TABLE IF NOT EXISTS functions (key INTEGER "
                      "PRIMARY KEY, function TEXT);";
  char *errMsg;
  if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::cerr << "Failed to create table: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  }
}

std::string Worker::serializeFunction(const FunctionType &function) {
  std::string serializedFunction = "function";
  return serializedFunction;
}

FunctionType
Worker::deserializeFunction(const std::string &serializedFunction) {
  FunctionType function = [](const std::string &signal) {
    std::cout << "Received signal: " << signal << std::endl;
  };
  return function;
}

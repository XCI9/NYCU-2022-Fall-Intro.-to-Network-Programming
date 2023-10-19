#pragma once
#include "DataBase.h"
#include "Header.h"
#include <charconv>
#include <regex>

namespace DNSServer {
class Server {
    DataBase m_database;
    int m_socketFd;

  public:
    Server(const int port, const char* configFilePath);

    void print_hex(const char* s, int n);

    std::size_t memcpyRecord(char* destination, const std::shared_ptr<Record>& record);

    void checkConnection();
};
} // namespace DNSServer

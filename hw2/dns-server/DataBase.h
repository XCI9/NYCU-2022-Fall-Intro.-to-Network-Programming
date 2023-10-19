#pragma once
#include <charconv>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <vector>

#include "DomainManager.h"
#include "Record.h"

namespace DNSServer {
class DataBase {
    DomainManager m_manager;
    std::string m_foreignServer;
    int m_foreignServerSocket;

    Record::Class classStr2Int(std::string_view classString);

    Record::Type typeStr2Int(std::string_view classString);

    void loadConfig(const char* path);

    void loadDomainInfo(const char* path);

  public:
    DataBase(const char* path);

    int getForeignServerSocket() const;

    DomainManager& operator()();
};
} // namespace DNSServer
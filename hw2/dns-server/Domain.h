#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Record.h"

namespace DNSServer {
class Domain {
    std::vector<std::shared_ptr<Record>> m_records;
    std::shared_ptr<Record> m_SOA;

  public:
    std::vector<std::shared_ptr<Record>> find(const std::string key, Record::Type QType, Record::Class QClass);

    std::vector<std::shared_ptr<Record>> find(const std::string key);

    std::vector<std::shared_ptr<Record>>& data();

    std::vector<std::shared_ptr<Record>> SOA();

    std::vector<std::shared_ptr<Record>> NS();

    void addRecord(std::string_view domainName, std::string_view name,
                   Record::Class classType, Record::Type type, uint32_t TTL, const std::string& data);
};

} // namespace DNSServer
#pragma once
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "DomainTree.h"
#include "Record.h"

namespace DNSServer {
class DomainManager {
    DomainTree m_tree;

  public:
    void insert(std::span<std::string_view> keys, const std::shared_ptr<Domain>& domain);

    std::shared_ptr<Domain>& getDomain(std::span<std::string_view> name);

    bool contains(std::span<std::string_view> name);

    std::vector<std::shared_ptr<Record>> getRecord(std::span<std::string_view> name, Record::Type QType, Record::Class QClass);

    std::vector<std::shared_ptr<Record>> getRecord(std::span<std::string_view> name);
};
} // namespace DNSServer
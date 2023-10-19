#pragma once
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "Domain.h"
#include "Record.h"

namespace DNSServer {

class DomainTree {
  protected:
    typedef std::map<std::string, DomainTree, std::less<void>> Children;

    Children m_children;
    std::shared_ptr<Domain> m_value;

  public:
    void operator=(const std::shared_ptr<Domain>& value);

    DomainTree& operator[](const std::string& key);
    DomainTree& operator[](std::string_view key);

    bool contains(const std::string& key) const;
    bool contains(std::string_view key) const;

    std::shared_ptr<Domain>& operator()();
    const std::shared_ptr<Domain>& operator()() const;

    // for range-base for loop
    auto begin() const;
    auto end() const;
};
} // namespace DNSServer
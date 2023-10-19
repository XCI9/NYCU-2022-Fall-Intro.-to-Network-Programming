#include "DomainTree.h"

using namespace DNSServer;

void DomainTree::operator=(const std::shared_ptr<Domain>& value) {
    m_value = value;
}

DomainTree& DomainTree::operator[](const std::string& key) { return m_children[key]; }

DomainTree& DomainTree::operator[](std::string_view key) {
    if (auto find{ m_children.find(key) }; find == m_children.end())
        return m_children[std::string{ key }];
    else
        return find->second;
}

bool DomainTree::contains(const std::string& key) const { return m_children.contains(key); }

bool DomainTree::contains(std::string_view key) const { return m_children.contains(key); }

std::shared_ptr<Domain>& DomainTree::operator()() { return m_value; }
const std::shared_ptr<Domain>& DomainTree::operator()() const { return m_value; }

// for range-base for loop
auto DomainTree::begin() const { return m_children.begin(); }
auto DomainTree::end() const { return m_children.end(); }

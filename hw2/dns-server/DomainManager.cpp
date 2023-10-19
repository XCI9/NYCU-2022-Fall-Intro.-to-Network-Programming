#include "DomainManager.h"

using namespace DNSServer;

void DomainManager::insert(std::span<std::string_view> keys, const std::shared_ptr<Domain>& domain) {
    auto* currentNode{ &m_tree };
    for (const auto& key : std::views::reverse(keys))
        currentNode = &(*currentNode)[key];

    (*currentNode)() = domain;
}

std::shared_ptr<Domain>& DomainManager::getDomain(std::span<std::string_view> name) {
    auto* currentNode{ &m_tree };
    for (const auto& key : std::views::reverse(name)) {
        if (currentNode->contains(key))
            currentNode = &(*currentNode)[key];
        else
            break;
    }
    return (*currentNode)();
}

bool DomainManager::contains(std::span<std::string_view> name) {
    return getDomain(name) != nullptr;
}

std::vector<std::shared_ptr<Record>> DomainManager::getRecord(std::span<std::string_view> name, Record::Type QType, Record::Class QClass) {
    auto* currentNode{ &m_tree };
    auto i{ std::ssize(name) - 1 };
    for (; i >= 0; i--) {
        if (currentNode->contains(name[i]))
            currentNode = &(*currentNode)[name[i]];
        else
            break;
    }
    if ((*currentNode)() == nullptr)
        return {};
    std::string key{ combine(name.first(i + 1)) };

    return (*currentNode)()->find(key, QType, QClass);
}

std::vector<std::shared_ptr<Record>> DomainManager::getRecord(std::span<std::string_view> name) {
    auto* currentNode{ &m_tree };
    auto i{ std::ssize(name) - 1 };
    for (; i >= 0; i--) {
        if (currentNode->contains(name[i]))
            currentNode = &(*currentNode)[name[i]];
        else
            break;
    }
    if ((*currentNode)() == nullptr)
        return {};
    std::string key{ combine(name.first(i + 1)) };

    return (*currentNode)()->find(key);
}

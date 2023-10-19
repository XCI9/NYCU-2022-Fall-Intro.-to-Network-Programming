#include "Domain.h"

using namespace DNSServer;

std::vector<std::shared_ptr<Record>> Domain::find(const std::string key, Record::Type QType, Record::Class QClass) {
    std::vector<std::shared_ptr<Record>> result;
    for (const auto& info : m_records) {
        if (info->name() == key && info->cls() == QClass && info->type() == QType)
            result.emplace_back(info);
    }
    return result;
}

std::vector<std::shared_ptr<Record>> Domain::find(const std::string key) {
    std::vector<std::shared_ptr<Record>> result;
    for (const auto& info : m_records) {
        if (info->name() == key)
            result.emplace_back(info);
    }
    return result;
}

std::vector<std::shared_ptr<Record>>& Domain::data() { return m_records; }

std::vector<std::shared_ptr<Record>> Domain::SOA() {
    std::vector<std::shared_ptr<Record>> result;
    for (const auto& info : m_records) {
        if (info->type() == Record::Type::SOA)
            result.emplace_back(info);
    }
    return result;
}

std::vector<std::shared_ptr<Record>> Domain::NS() {
    std::vector<std::shared_ptr<Record>> result;
    for (const auto& info : m_records) {
        if (info->type() == Record::Type::NS)
            result.emplace_back(info);
    }
    return result;
}

void Domain::addRecord(std::string_view domainName, std::string_view name,
                       Record::Class classType, Record::Type type, uint32_t TTL, const std::string& data) {
    switch (type) {
    case Record::Type::A:
        m_records.emplace_back(std::make_shared<ARecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    case Record::Type::AAAA:
        m_records.emplace_back(std::make_shared<AAAARecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    case Record::Type::NS:
        m_records.emplace_back(std::make_shared<NSRecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    case Record::Type::CNAME:
        m_records.emplace_back(std::make_shared<CNameRecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    case Record::Type::SOA:
        m_SOA = std::make_shared<SOARecord>(domainName, name, classType, TTL, data.c_str());
        m_records.emplace_back(m_SOA);
        break;
    case Record::Type::MX:
        m_records.emplace_back(std::make_shared<MXRecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    case Record::Type::TXT:
        m_records.emplace_back(std::make_shared<TXTRecord>(domainName, name, classType, TTL, data.c_str()));
        break;
    default:
        printf("unknown type!\n");
    }
}
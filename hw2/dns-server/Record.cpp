#include "Record.h"

using namespace DNSServer;

std::string Record::string2label(const char* str) const {
    char output[1024]{};
    char* pos{ output };

    for (std::string_view splitter{ "." }, lineView{ str }; const auto split : std::views::split(lineView, splitter)) {
        std::string_view dataPart{ split.begin(), split.end() };
        uint8_t length{ static_cast<uint8_t>(dataPart.size()) };
        pos += memcpyN(pos, &length, sizeof(length));
        pos += sprintf(pos, "%.*s", static_cast<int>(dataPart.length()), dataPart.data());
    }
    pos += 1;

    return std::string{ output, pos };
}

Record::Record(std::string_view domainName, std::string_view name, Class classType, Type type, uint32_t ttl) : m_name{ name },
                                                                                                               m_class{ classType },
                                                                                                               m_type{ type },
                                                                                                               m_TTL{ ttl } {
    if (domainName.back() == '.')
        domainName.remove_suffix(1);
    if (m_name == "")
        m_fullName = domainName;
    else
        m_fullName = m_name + "." + std::string{ domainName };
}

std::string_view Record::name() const { return m_name; }

std::string_view Record::fullName() const { return m_fullName; }

Record::Type Record::type() const { return m_type; }

Record::Class Record::cls() const { return m_class; }

uint32_t Record::TTL() const { return m_TTL; }

/************************************************************************************************************************************/
ARecord::ARecord(std::string_view domainName, std::string_view name,
                 Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::A, ttl },
                                                                     m_address{ RData } {}

std::string ARecord::getAsRData() const {
    char rdata[1024];
    char* pos{ rdata };

    uint32_t addressInt{ inet_addr(m_address.c_str()) };
    pos += memcpyN(pos, &addressInt, sizeof(addressInt));

    return std::string{ rdata, pos };
}

std::string_view ARecord::getAdditionalKey() const {
    return m_address;
}

/************************************************************************************************************************************/
AAAARecord::AAAARecord(std::string_view domainName, std::string_view name,
                       Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::AAAA, ttl },
                                                                           m_address{ RData } {}

std::string AAAARecord::getAsRData() const {
    char rdata[16]{};

    inet_pton(AF_INET6, m_address.c_str(), &rdata);

    return std::string{ rdata, rdata + 16 };
}

std::string_view AAAARecord::getAdditionalKey() const {
    return m_address;
}

/************************************************************************************************************************************/

NSRecord::NSRecord(std::string_view domainName, std::string_view name,
                   Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::NS, ttl },
                                                                       m_nsdName{ RData } {}

std::string NSRecord::getAsRData() const {
    return string2label(m_nsdName.c_str());
}

std::string_view NSRecord::getAdditionalKey() const {
    return m_nsdName;
}

/************************************************************************************************************************************/

CNameRecord::CNameRecord(std::string_view domainName, std::string_view name,
                         Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::CNAME, ttl },
                                                                             m_nsdName{ RData } {}

std::string CNameRecord::getAsRData() const {
    return string2label(m_nsdName.c_str());
}

std::string_view CNameRecord::getAdditionalKey() const {
    return m_nsdName;
}

/************************************************************************************************************************************/

MXRecord::MXRecord(std::string_view domainName, std::string_view name,
                   Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::MX, ttl } {
    char buffer[1024]{};
    sscanf(RData, "%hd %s", &m_preference, buffer);
    m_exchange = buffer;
}

std::string MXRecord::getAsRData() const {
    char rdata[1024]{};
    char* pos{ rdata };

    auto preferenceNetEndian{ htons(m_preference) };
    pos += memcpyN(pos, &preferenceNetEndian, sizeof(preferenceNetEndian));

    std::string labelString{ string2label(m_exchange.c_str()) };
    pos += memcpyN(pos, labelString.c_str(), labelString.size());

    return std::string{ rdata, pos };
}

std::string_view MXRecord::getAdditionalKey() const {
    return m_exchange;
}

/************************************************************************************************************************************/

SOARecord::SOARecord(std::string_view domainName, std::string_view name,
                     Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::SOA, ttl } {
    char mnameTemp[64]{};
    char rnameTemp[64]{};
    sscanf(RData, "%s %s %u %u %u %u %u", mnameTemp, rnameTemp, &m_serial, &m_refresh, &m_retry, &m_expire, &m_minimum);
    m_mname = mnameTemp;
    m_rname = rnameTemp;
    if (m_mname.back() == '.')
        m_mname.pop_back();
    if (m_rname.back() == '.')
        m_rname.pop_back();
}

std::string SOARecord::getAsRData() const {
    char rdata[1024];
    char* pos{ rdata };

    std::string mnameLabelString{ string2label(m_mname.c_str()) };
    std::string rnameLabelString{ string2label(m_rname.c_str()) };
    pos += memcpyN(pos, mnameLabelString.c_str(), mnameLabelString.size());
    pos += memcpyN(pos, rnameLabelString.c_str(), rnameLabelString.size());

    auto serialNetEndian{ htonl(m_serial) };
    pos += memcpyN(pos, &serialNetEndian, sizeof(serialNetEndian));

    auto refreshNetEndian{ htonl(m_refresh) };
    pos += memcpyN(pos, &refreshNetEndian, sizeof(refreshNetEndian));

    auto retryNetEndian{ htonl(m_retry) };
    pos += memcpyN(pos, &retryNetEndian, sizeof(retryNetEndian));

    auto expireNetEndian{ htonl(m_expire) };
    pos += memcpyN(pos, &expireNetEndian, sizeof(expireNetEndian));

    auto minimumNetEndian{ htonl(m_minimum) };
    pos += memcpyN(pos, &minimumNetEndian, sizeof(minimumNetEndian));

    return std::string{ rdata, pos };
}

std::string_view SOARecord::getAdditionalKey() const {
    return std::string_view{};
}

/************************************************************************************************************************************/

TXTRecord::TXTRecord(std::string_view domainName, std::string_view name,
                     Class classType, uint32_t ttl, const char* RData) : Record{ domainName, name, classType, Type::TXT, ttl },
                                                                         m_nsdName{ RData } {
}

std::string TXTRecord::getAsRData() const {
    return m_nsdName;
}

std::string_view TXTRecord::getAdditionalKey() const {
    return m_nsdName;
}

#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ranges>
#include <string>
#include <string_view>
#include <sys/socket.h>

#include "StringOperation.h"

namespace DNSServer {
class Record {
  public:
    enum class Class : uint16_t {
        IN = 1,
        CS = 2,
        CH = 3,
        HS = 4,
    };

    enum class Type : uint16_t {
        A = 1,
        AAAA = 28,
        NS = 2,
        CNAME = 5,
        SOA = 6,
        MX = 15,
        TXT = 16,
    };

  protected:
    std::string m_fullName;
    std::string m_name;
    Class m_class;
    Type m_type;
    uint32_t m_TTL;

    std::string string2label(const char* str) const;

  public:
    Record(std::string_view domainName, std::string_view name, Class classType, Type type, uint32_t ttl);

    virtual std::string getAsRData() const = 0;

    virtual std::string_view getAdditionalKey() const = 0;

    std::string_view name() const;

    std::string_view fullName() const;

    Type type() const;

    Class cls() const;

    uint32_t TTL() const;
};

class ARecord : public Record {
    std::string m_address;

  public:
    ARecord(std::string_view domainName, std::string_view name,
            Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class AAAARecord : public Record {
    std::string m_address;

  public:
    AAAARecord(std::string_view domainName, std::string_view name,
               Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class NSRecord : public Record {
    std::string m_nsdName;

  public:
    NSRecord(std::string_view domainName, std::string_view name,
             Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class CNameRecord : public Record {
    std::string m_nsdName;

  public:
    CNameRecord(std::string_view domainName, std::string_view name,
                Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class MXRecord : public Record {
    uint16_t m_preference;
    std::string m_exchange;

  public:
    MXRecord(std::string_view domainName, std::string_view name,
             Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class SOARecord : public Record {
    std::string m_mname;
    std::string m_rname;
    uint32_t m_serial;
    uint32_t m_refresh;
    uint32_t m_retry;
    uint32_t m_expire;
    uint32_t m_minimum;

  public:
    SOARecord(std::string_view domainName, std::string_view name,
              Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};

class TXTRecord : public Record {
    std::string m_nsdName;

  public:
    TXTRecord(std::string_view domainName, std::string_view name,
              Class classType, uint32_t ttl, const char* RData);

    std::string getAsRData() const override;

    std::string_view getAdditionalKey() const override;
};
} // namespace DNSServer
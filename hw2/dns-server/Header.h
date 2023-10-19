#pragma once
#include <netinet/in.h>

#pragma pack(1)

namespace DNSServer {
struct Header {
    uint16_t id;
    union {
        struct {
            uint16_t QR : 1;
            uint16_t opcode : 4;
            uint16_t AA : 1;
            uint16_t TC : 1;
            uint16_t RD : 1;
            uint16_t RA : 1;
            uint16_t Z : 3;
            uint16_t Rcode : 4;
        };
        uint16_t flags;
    };
    uint16_t QDCount;
    uint16_t ANCount;
    uint16_t NSCount;
    uint16_t ARCount;

    void setQR(uint16_t value);
    void setOpcode(uint16_t value);
    void setAA(uint16_t value);
    void setTC(uint16_t value);
    void setRD(uint16_t value);
    void setRA(uint16_t value);
    void setZ(uint16_t value);
    void setRCode(uint16_t value);

    void ntoh();
    void hton();
};
} // namespace DNSServer
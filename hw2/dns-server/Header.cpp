#include "Header.h"

using namespace DNSServer;

void Header::setQR(uint16_t value) {
    flags &= ~(1 << 15);
    flags |= value << 15;
}

void Header::setOpcode(uint16_t value) {
    flags &= ~(0b1111 << 11);
    flags |= value << 11;
}

void Header::setAA(uint16_t value) {
    flags &= ~(1 << 10);
    flags |= value << 10;
}

void Header::setTC(uint16_t value) {
    flags &= ~(1 << 9);
    flags |= value << 9;
}

void Header::setRD(uint16_t value) {
    flags &= ~(1 << 8);
    flags |= value << 8;
}

void Header::setRA(uint16_t value) {
    flags &= ~(1 << 7);
    flags |= value << 7;
}

void Header::setZ(uint16_t value) {
    flags &= ~(0b111 << 4);
    flags |= value << 4;
}

void Header::setRCode(uint16_t value) {
    flags &= ~0b1111;
    flags |= value;
}

void Header::ntoh() {
    id = ntohs(id);
    flags = ntohs(flags);
    QDCount = ntohs(QDCount);
    ANCount = ntohs(ANCount);
    NSCount = ntohs(NSCount);
    ARCount = ntohs(ARCount);
}

void Header::hton() {
    id = htons(id);
    flags = htons(flags);
    QDCount = htons(QDCount);
    ANCount = htons(ANCount);
    NSCount = htons(NSCount);
    ARCount = htons(ARCount);
}

#include <Zipper/Zipper.hpp>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVERIP "6.6.6.6"
#define CLIENTIP "8.8.8.8"
#define PROTOCOL 161

#pragma pack(1)

#define MAXLINE 1475 // MTU(1500) - sizeof(ip)(23)  - seq.(2)

struct DataPacket {
    ip header{};
    unsigned short id{ 0 };
    unsigned char data[MAXLINE];
    bool valid{ false };
    int length;
};

struct ACKPacket {
    ip header{};
    unsigned short id{ 0 };
};

unsigned short cksum(void* in, int sz) {
    long sum = 0;
    unsigned short* ptr = (unsigned short*)in;

    for (; sz > 1; sz -= 2)
        sum += *ptr++;
    if (sz > 0)
        sum += *((unsigned char*)ptr);
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

void setIPHeader(ip* header, const char* broadcastIP) {
    header->ip_v = 4;
    header->ip_src.s_addr = inet_addr(CLIENTIP);
    header->ip_dst.s_addr = inet_addr(broadcastIP);
    header->ip_len = sizeof(ip) + 16;
    header->ip_p = PROTOCOL;
    header->ip_ttl = 100;
    header->ip_hl = sizeof(ip) >> 2;
    header->ip_sum = cksum(header, sizeof(header));
}

static int s = -1; /* socket */

void recvfrom_alarm(int _) {}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./client <path-to-read-files> <total-number-of-files> <broadcast-address>\n");
        return -1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);

    if ((s = socket(AF_INET, SOCK_RAW, PROTOCOL)) < 0) {
        fprintf(stderr, "s\n");
        return -1;
    }
    printf("[client] argv: %s %s %s\n", argv[1], argv[2], argv[3]);

    // turn on broadcast
    const int on{ 1 };
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    signal(SIGALRM, recvfrom_alarm);

    // turn on ip header
    setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

    sockaddr_in broadcastAddress{};
    broadcastAddress.sin_family = AF_INET;
    // broadcastAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); // broadcast address
    inet_pton(AF_INET, argv[3], &broadcastAddress.sin_addr);

    printf("[client] start zipping file.\n");
    using namespace zipper;
    // zip file
    Zipper zipper("all.zip", Zipper::openFlags::Overwrite);
    zipper.add(argv[1]); // argv[1] = path-to-read-files
    zipper.close();
    printf("[client] zip file success\n");

    std::vector<DataPacket> data(10000);
    // file to packets
    DataPacket packet;
    setIPHeader(&packet.header, argv[3]);
    FILE* file{ fopen("all.zip", "rb") };
    int readCount;
    while ((readCount = fread(packet.data, 1, MAXLINE, file)) != 0) {
        packet.length = readCount + 2 + sizeof(ip); // 2 is file number
        data[packet.id] = packet;
        packet.id++;
    }
    // last packet is full
    if (packet.length == MAXLINE + 2 + sizeof(ip)) {
        packet.length = 2 + sizeof(ip);
        data[packet.id] = packet;
    }
    fclose(file);

    printf("[client] %d packets in total, start sending...\n", packet.id);

    int currentPacketNumber{ 0 };
    while (1) {
        // printf("[client] sending packet %d\n", currentPacketNumber);

        if (sendto(s, &data[currentPacketNumber], data[currentPacketNumber].length, 0, (sockaddr*)&broadcastAddress, sizeof(broadcastAddress)) < 0)
            perror("[client] send fail");

        while (1) {
            ACKPacket ack;
            sockaddr_in receiveAddr;
            socklen_t len{ sizeof(receiveAddr) };
            if (recvfrom(s, &ack, sizeof(ack), 0, (sockaddr*)&receiveAddr, &len) < 0)
                perror("[client] receive fail!");

            ip* receiveHeader{ &ack.header };
            if (receiveHeader->ip_src.s_addr == inet_addr(SERVERIP)) {
                if (ack.id == 65534)
                    return 0;
                // printf("[client] ACK %hd\n", ack.id);
                break;
            }
        }

        currentPacketNumber++;
    }
    return 0;
}
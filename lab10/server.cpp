#include <Zipper/Unzipper.hpp>
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
#include <vector>

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
    header->ip_src.s_addr = inet_addr(SERVERIP);
    header->ip_dst.s_addr = inet_addr(broadcastIP);
    header->ip_len = sizeof(ip) + 16;
    header->ip_p = PROTOCOL;
    header->ip_ttl = 100;
    header->ip_hl = sizeof(ip) >> 2;
    header->ip_sum = cksum(header, sizeof(header));
}

void recvfrom_alarm(int _) {}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./ping <address>.\n");
        return -1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[server] argv: %s %s %s\n", argv[1], argv[2], argv[3]);

    int sockedFd;
    if ((sockedFd = socket(AF_INET, SOCK_RAW, PROTOCOL)) < 0) {
        fprintf(stderr, "s\n");
        return -1;
    }

    sockaddr_in broadcastAddress{};
    broadcastAddress.sin_family = AF_INET;
    // broadcastAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); // broadcast address
    inet_pton(AF_INET, argv[3], &broadcastAddress.sin_addr);

    // turn on broadcast
    const int on{ 1 };
    setsockopt(sockedFd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    signal(SIGALRM, recvfrom_alarm);

    // turn on ip header
    setsockopt(sockedFd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

    std::vector<unsigned char> zipData;
    zipData.reserve(10000 * 1470);

    printf("[server] ready for receive.\n");

    sockaddr_in cliaddr{};
    std::vector<DataPacket> packetQueue(10000);
    DataPacket receivePacket;
    receivePacket.valid = true;
    // send
    ACKPacket ack;
    setIPHeader(&ack.header, argv[3]);
    while (1) {
        int receiveCount;

        socklen_t len{ sizeof(cliaddr) };
        while ((receiveCount = recvfrom(sockedFd, &receivePacket, MAXLINE + sizeof(ip) + 2, 0, (sockaddr*)&cliaddr, &len)) <= 0) {
        }

        if (receivePacket.header.ip_src.s_addr == inet_addr(SERVERIP))
            continue;

        receivePacket.length = receiveCount - sizeof(ip) - 2;
        packetQueue[receivePacket.id] = receivePacket;
        if (receivePacket.length != MAXLINE) {
            // printf("[server] receive finish!\n");
            break;
        }
        // printf("[server] receive packet %hd!\n", receivePacket.id);

        // send ACK
        ack.id = receivePacket.id;
        if (sendto(sockedFd, &ack, sizeof(ack), 0, (sockaddr*)&broadcastAddress, sizeof(broadcastAddress)) < 0)
            perror("[server] send error");
        // printf("[server] send ACK %hd!\n", receivePacket.id);
    }

    printf("[server] receive finish...\n");
    for (int i{ 0 }; i <= receivePacket.id; i++) {
        zipData.insert(zipData.end(), std::begin(packetQueue[i].data), std::end(packetQueue[i].data));
    }

    using namespace zipper;
    printf("[server] unzipping file...\n");
    Unzipper unzipper(zipData);
    unzipper.extractAll(true);
    unzipper.close();
    printf("[server] finish!\n");

    // Fin
    ack.id = 65534;
    sendto(sockedFd, &ack, sizeof(ack), 0, (sockaddr*)&broadcastAddress, sizeof(broadcastAddress));
    printf("[server] send FIN\n");

    return 0;
}
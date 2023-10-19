#include <Zipper/Zipper.hpp>
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#pragma pack(1)

#define MAXLINE 600

constexpr int MAXSENDCOUNT{ 1000 };

struct ReplyPacket {
    unsigned short id{ 0 };
    unsigned short finishPacket{ 0 };
};

// struct ReplyPacket {
//     unsigned short id{ 0 };
// };

struct SendPacket {
    unsigned short id{ 0 };
    char data[MAXLINE + 1];
    int length;
};

struct Packet {
    int id;
    char data[MAXLINE + 1 + 4];
    int length;
    unsigned long long sendTime{ 0 };
    bool isFinish{ false };
};

unsigned long long getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ull + tv.tv_usec;
}

void dg_cli(int sockfd, const sockaddr* pservaddr, socklen_t servlen) {
    int n;

    // struct timeval tv = { 0, 220'000 };
    // struct timeval tv = { 0, 10 };
    // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    connect(sockfd, (sockaddr*)pservaddr, servlen);

    // record all data
    std::map<int, SendPacket> data;

    SendPacket packet;
    packet.id = 0;

    FILE* file = fopen("all.zip", "rb");

    int readCount;
    while ((readCount = fread(packet.data, 1, MAXLINE, file)) != 0) {
        packet.length = readCount;
        // add file number
        packet.length += 2;

        data[packet.id] = packet;

        packet.id++;
    }

    // last packet is full
    if (packet.length == MAXLINE + 2) {
        packet.length = 2;
        data[packet.id] = packet;
        packet.id++;
    }
    fclose(file);

    int packetCount{ data.size() };
    fprintf(stderr, "[client] packet count: %d\n", packetCount);

    // static constexpr const unsigned long long timeout{ 300'000 }; // 200ms
    ReplyPacket serverReply;
    int totalSendPacketCount{ 0 };
    int round{ 0 };
    unsigned long long lastRoundTime{ getTimeStamp() };
    while (data.size() > 0) {
        unsigned long long timeLast{ getTimeStamp() - lastRoundTime };
        // if (timeLast < 200'000)
        //     usleep(200'000 - timeLast);
        lastRoundTime = getTimeStamp();
        int i{ 0 };
        for (auto it{ data.begin() }; it != data.end(); it++) {
            if (i % 10 == 0) {
                while ((n = recv(sockfd, &serverReply, sizeof(serverReply), MSG_DONTWAIT)) > 0) {
                    int acceptPacketId{ serverReply.id };
                    // auto acceptPacket{ data.find(acceptPacketId) };
                    // if (acceptPacket != data.end()) {
                    //     if (acceptPacket == it)
                    //         it = data.erase(acceptPacket);
                    //     else
                    //         data.erase(acceptPacket);
                    // }
                    if (acceptPacketId == 65534)
                        return;

                    auto currentIt{ data.find(acceptPacketId) };
                    if (currentIt != data.end())
                        if (currentIt == it)
                            it = data.erase(currentIt);
                        else
                            data.erase(currentIt);

                    for (int i{ 0 }; i < 16; i++) {
                        if (serverReply.finishPacket & 1 << i == 1) {
                            auto currentIt{ data.find(acceptPacketId - i - 1) };
                            if (currentIt != data.end())
                                if (currentIt == it)
                                    it = data.erase(currentIt);
                                else
                                    data.erase(currentIt);
                        }
                    }

                    fprintf(stderr, "[client] accept packet: %d\n", acceptPacketId);
                }
                fprintf(stderr, "[client] last packet count: %lu\n", data.size());
            }

            write(sockfd, (char*)&it->second, it->second.length);
            // fprintf(stderr, "[client] send packet %d\n", it->second.id);
            totalSendPacketCount++;
            if (totalSendPacketCount == 500) {
                totalSendPacketCount = 0;
                usleep(200'000);
            }

            i++;

            if (data.size() == 0)
                break;
        }
    }
    fprintf(stderr, "[client] packet count: %d\n", packetCount);
    fprintf(stderr, "[client] total send packet count: %d\n", totalSendPacketCount);
}

int main(int argc, char** argv) {

    int serverFd;
    sockaddr_in server{};

    if (argc != 5) {
        fprintf(stderr, "usage: /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>\n");
        exit(0);
    }

    std::string fileRootPath{ argv[1] };
    int totalFileCount{ std::stoi(argv[3]) };

    server.sin_family = AF_INET;
    server.sin_port = htons(std::stoi(argv[3]));
    inet_pton(AF_INET, argv[4], &server.sin_addr);

    serverFd = socket(AF_INET, SOCK_DGRAM, 0);

    int n = 500 * 1024; // 500KB
    setsockopt(serverFd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
    n = 500 * 1024; // 500KB
    setsockopt(serverFd, SOL_SOCKET, SO_SNDBUF, &n, sizeof(n));

    using namespace zipper;
    // zip file
    Zipper zipper("all.zip", Zipper::openFlags::Overwrite);
    for (int i{ 0 }; i < 1000; i++) {
        char sendline[MAXLINE];
        char fileName[12];
        sprintf(fileName, "%06d", i);

        zipper.add((fileRootPath + "/" + fileName).c_str());
    }
    zipper.close();
    fprintf(stderr, "zip file success\n");

    dg_cli(serverFd, (sockaddr*)&server, sizeof(server));
    usleep(100'000);
    exit(0);
}
#include <Zipper/Unzipper.hpp>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <netinet/in.h>
#include <set>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#pragma pack(1)

#define MAXLINE 600

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
    bool valid;
    int length;
};

struct Packet {
    bool valid{ false };
    int id;
    char data[MAXLINE + 1 + 4];
    int length;
};

void dg_echo(int sockfd, std::string fileRootPath, sockaddr* pcliaddr, socklen_t clilen) {
    int n;
    socklen_t len;

    int currentFileId{ 0 };
    char fileName[12];
    sprintf(fileName, "%06d", currentFileId++);
    FILE* file = fopen("all.zip", "wb");

    std::deque<SendPacket> packetQueue;
    int firstPacketId{ 0 };
    bool isFinish{ false };
    int totalReceivePacketCount{ 0 };
    while (1) {
        // receive packet
        SendPacket receivePacket;
        receivePacket.valid = true;

        len = clilen;
        int receiveCount{ recvfrom(sockfd, &receivePacket, MAXLINE + 2, 0, pcliaddr, &len) };
        if (receiveCount <= 0)
            continue;
        ((char*)&receivePacket)[receiveCount] = '\0';

        // fprintf(stderr, "[server] recv packet %d\n", receivePacket.id);
        totalReceivePacketCount++;

        ReplyPacket reply;
        reply.id = receivePacket.id;
        int currentPacketIndex{ receivePacket.id - firstPacketId };

        // set finish bit
        reply.finishPacket = 0;
        for (int i{ currentPacketIndex - 1 }; i > currentPacketIndex - 17; i--) {
            if (i < 0)
                reply.finishPacket |= 1 << (i - currentPacketIndex - 1);

            else if (i > packetQueue.size() || packetQueue[i].valid)
                reply.finishPacket |= 1 << (i - currentPacketIndex - 1);
        }
        // if (currentPacketIndex >= 0) {
        //     int upperbound{ std::min(currentPacketIndex + 16, static_cast<int>(packetQueue.size())) };
        //     for (int i{ currentPacketIndex }; i < upperbound; i++)
        //         if (packetQueue[i].valid)
        //             reply.finishPacket |= 1 << (i - currentPacketIndex);
        // }

        for (int i{ 0 }; i < 3; i++)
            sendto(sockfd, &reply, sizeof(reply), 0, pcliaddr, len);

        if (receivePacket.id < firstPacketId)
            continue;
        // already exist
        if (currentPacketIndex < packetQueue.size() && packetQueue[currentPacketIndex].valid == true) {
            continue;
        } else {
            while (currentPacketIndex >= packetQueue.size())
                packetQueue.push_back(SendPacket{});

            // put new receive packet into deque
            receivePacket.length = receiveCount - 2;

            packetQueue[receivePacket.id - firstPacketId] = receivePacket;

            // check if deque front is finish
            while (!packetQueue.empty() && packetQueue.front().valid) {
                SendPacket currentPacket{ packetQueue.front() };
                packetQueue.pop_front();
                firstPacketId++;

                if (currentPacket.length != MAXLINE) {
                    fwrite(currentPacket.data, currentPacket.length, 1, file);
                    // fprintf(stderr, "[server] end of file no.%d\n", currentFileId);
                    fclose(file);

                    isFinish = true;
                    break;

                } else {
                    fwrite(currentPacket.data, currentPacket.length, 1, file);
                }
            }
            if (isFinish)
                break;
        }
    }
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        fprintf(stderr, "usage: /server <path-to-store-files> <total-number-of-files> <port>\n");
        exit(0);
    }
    fprintf(stderr, "[server]%s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);

    int sockfd;
    sockaddr_in servaddr{};
    sockaddr_in cliaddr{};

    std::string fileRootPath{ argv[1] };
    int totalFileCount{ std::stoi(argv[2]) };

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(std::stoi(argv[3]));

    bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));

    dg_echo(sockfd, fileRootPath, (sockaddr*)&cliaddr, sizeof(cliaddr));

    using namespace zipper;
    fprintf(stderr, "[server]unzipping file...\n");
    Unzipper unzipper("all.zip");
    unzipper.extractAll(fileRootPath, true);
    unzipper.close();
    fprintf(stderr, "[server]finish!\n");

    ReplyPacket reply;
    reply.id = 65534;
    int len = sizeof(cliaddr);
    for (int i{ 0 }; i < 5; i++)
        sendto(sockfd, &reply, sizeof(reply), 0, (sockaddr*)&cliaddr, len);
}
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXLINE 204800
#define MAXSEND 16300
#define SOCKETCOUNT 19
int socketList[SOCKETCOUNT];
int cmdSocket;

void handler(int signal) {

    for (int i{ 0 }; i < SOCKETCOUNT; i++) {
        close(socketList[i]);
    }

    char recvline[MAXLINE]{};
    if (read(cmdSocket, recvline, MAXLINE) != 0) {
        printf("%s", recvline);
    }
    write(cmdSocket, "/report\n", 8);
    char recvline1[MAXLINE]{};
    if (read(cmdSocket, recvline1, MAXLINE) != 0) {
        printf("%s\n", recvline1);
    }

    close(cmdSocket);
    exit(0);
}

char sendline[MAXLINE]{};
void sendMsg() {
    while (1) {
        // sendline
        for (int i{ 0 }; i < SOCKETCOUNT; i++) {
            write(socketList[i], sendline, MAXSEND);
        }
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    int port{ std::stoi(std::string{ argv[2] }) };

    cmdSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(cmdSocket, (sockaddr*)&servaddr, sizeof(servaddr));

    for (int i{ 0 }; i < SOCKETCOUNT; i++) {
        socketList[i] = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port + 1);
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

        connect(socketList[i], (sockaddr*)&servaddr, sizeof(servaddr));
    }

    write(cmdSocket, "/reset\n", 7);
    sendMsg();

    char recvline[MAXLINE]{};

    exit(0);
}

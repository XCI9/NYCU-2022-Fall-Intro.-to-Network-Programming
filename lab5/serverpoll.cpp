#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <ranges>
#include <string_view>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 20
#define LISTENQ 1024

struct ClientInfo {
    std::string name;
    std::string ip;
    int port;
};

std::string randomString(const int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string output;
    output.reserve(length);

    for (int i{ 0 }; i < length; i++)
        output += alphanum[rand() % (sizeof(alphanum) - 1)];

    return output;
}

std::string getTimeString() {
    std::time_t rawtime;
    std::tm *timeinfo;
    char buffer[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// broadcast msg to all user except exceptUser with sender name senderName, if senderName is not set, then default to server send
void broadcast(pollfd *client, const int maxIndex, const int exceptUser, const std::string &msg, const std::string &senderName = "\n") {
    std::string sendMsg{ getTimeString() };
    if (senderName == "\n") // '\n' means server
        sendMsg += " *** ";
    else
        sendMsg += " <" + senderName + "> ";
    sendMsg += msg;

    for (int i{ 1 }; i <= maxIndex; i++) {
        int userFd{ client[i].fd };

        // broadcast to all client except the client that send this msg
        if (userFd < 0 || userFd == exceptUser)
            continue;

        send(userFd, sendMsg.c_str(), sendMsg.length(), MSG_NOSIGNAL);
    }
}

void sendMsg(const int sockfd, const std::string &input) {
    std::string msg{ getTimeString() + " *** " + input };
    send(sockfd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
}

void listAllUser(const int sockfd, pollfd *client, const int maxIndex, ClientInfo *clientInfo) {
    std::string msg{ "-------------------------------------------\n" };
    for (int i{ 1 }; i <= maxIndex; i++) {
        int userFd{ client[i].fd };
        if (userFd < 0)
            continue;

        if (userFd == sockfd)
            msg += "* ";
        else
            msg += "  ";
        msg += clientInfo[i].name + "\t" + clientInfo[i].ip + ":" + std::to_string(clientInfo[i].port) + "\n";
    }
    msg += "-------------------------------------------\n";
    write(sockfd, msg.c_str(), msg.length());
}

int main(int argc, char **argv) {
    srand(time(NULL));
    int listenfd{ socket(AF_INET, SOCK_STREAM, 0) };

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(10005);

    bind(listenfd, (sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    int totalUserCount{ 0 };
    int maxIndex{ 0 }; /* index into client[] array */
    ClientInfo clientInfo[FD_SETSIZE]{};
    pollfd client[FD_SETSIZE];                             // record which descriptor is available
    std::ranges::for_each(client, [](pollfd &client) { client.fd = -1; client.events = POLLRDNORM | POLLHUP; }); // init set all to -1
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;

    while (1) {
        int nready{ poll(client, maxIndex + 1, -1) };

        if (client[0].revents & POLLRDNORM) { // check if new connection
            sockaddr_in clientAddr;
            socklen_t clientLength{ sizeof(clientAddr) };
            int connfd{ accept(listenfd, (sockaddr *)&clientAddr, &clientLength) };

            std::string ip{ inet_ntoa(clientAddr.sin_addr) };
            int port{ ntohs(clientAddr.sin_port) };
            printf("* client connected from %s:%d\n", ip.c_str(), port);

            int i{ 0 };
            for (; i < FD_SETSIZE; i++) // find an empty place to put new connection
                if (client[i].fd < 0) {
                    client[i].fd = connfd; /* save descriptor */

                    clientInfo[i].port = port;
                    clientInfo[i].ip = ip;
                    clientInfo[i].name = randomString(5);

                    if (i > maxIndex)
                        maxIndex = i; /* max index in client[] array */
                    totalUserCount++;

                    sendMsg(connfd, "Welcome to the simple CHAT server\n");
                    sendMsg(connfd, "Total " + std::to_string(totalUserCount) + " users online now. Your name is <" + clientInfo[i].name + ">\n");
                    broadcast(client, maxIndex, connfd, "User <" + clientInfo[i].name + "> has just landed on the server\n");
                    break;
                }
            if (i == FD_SETSIZE)
                fprintf(stderr, "too many clients!\n");
            --nready;
        }

        if (nready <= 0) // nothing available
            continue;

        for (int i{ 1 }; i <= maxIndex; i++) { /* check all clients for data */
            int currentClient{ client[i].fd };
            if (currentClient < 0)
                continue;

            if (client[i].revents & (POLLRDNORM | POLLHUP)) { // if receive data
                char buf[MAXLINE + 1]{};
                ssize_t readDataCount{ read(currentClient, buf, MAXLINE) };
                if (readDataCount == 0) {
                    // connection closed by client
                    close(currentClient);

                    printf("* client %s:%d disconnected.\n", clientInfo[i].ip.c_str(), clientInfo[i].port);
                    broadcast(client, maxIndex, currentClient, "User <" + clientInfo[i].name + "> has left the server\n");

                    totalUserCount--;
                    client[i].fd = -1;
                } else {
                    buf[readDataCount] = '\0';
                    std::string_view input{ buf };

                    if (input.starts_with("/name ")) {
                        input.remove_suffix(1); // remove '\n'
                        input.remove_prefix(6); // remove "/name "

                        std::string oldName{ clientInfo[i].name };
                        clientInfo[i].name = input;

                        sendMsg(currentClient, "Nickname changed to <" + clientInfo[i].name + ">\n");

                        broadcast(client, maxIndex, currentClient, "User <" + oldName + "> renamed to <" + clientInfo[i].name + ">\n");
                    } else if (input == "/who\n") {
                        listAllUser(currentClient, client, maxIndex, clientInfo);
                    } else if (input.starts_with("/")) {
                        sendMsg(currentClient, "Unknown command!\n");
                    } else { // normal msg
                             //  for all online user, send msg
                        broadcast(client, maxIndex, currentClient, buf, clientInfo[i].name);
                    }
                }

                if (--nready <= 0)
                    break; /* no more readable descriptors */
            }
        }
    }
}

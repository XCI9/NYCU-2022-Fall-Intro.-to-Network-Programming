#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <poll.h>

#define LISTENQ 1024

void executeCmd(const int sockfd, const std::string& cmd) {
    dup2(sockfd, STDIN_FILENO);    //override stdin with socket
    dup2(sockfd, STDOUT_FILENO);   //override stdout with socket
    system(cmd.c_str());           //run cmd
}

int main(int argc, char** argv) {
    if(argc < 3)
        fprintf(stderr, "usage: ./nkat <port> <cmd>\n");
    uint16_t port{ static_cast<uint16_t>(std::stoi(argv[1])) };

    std::string cmd{};
    //concatenate all argv
    for(int i{ 2 }; i < argc; i++)
        cmd += std::string{argv[i]} + " ";
    cmd.pop_back(); //remove last space

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //convert endian
    serverAddr.sin_port = htons(port);

    int listenfd { socket(AF_INET, SOCK_STREAM, 0) }; //IPv4, TCP
    bind(listenfd, (sockaddr*)&serverAddr, sizeof(serverAddr));

    pollfd receivePollfd{};
    receivePollfd.fd = listenfd;
    receivePollfd.events = POLLRDNORM;

    listen(listenfd, LISTENQ);  //listen to listenfd, with queue size LISTENQ
    while (1) {
        poll(&receivePollfd, 1, 100); //check every 100ms, with one only fd
        
        //new connection
        if(receivePollfd.revents & POLLRDNORM){
            sockaddr_in clientAddr;
            socklen_t cleintlen{ sizeof(clientAddr) };

            int connectfd{ accept(listenfd, (sockaddr *)&clientAddr, &cleintlen) };
            
            char ip[20]{};
            inet_ntop(AF_INET, &clientAddr, ip, 20);
            printf("New connection from %s:%d\n", ip, ntohs(clientAddr.sin_port));

            if (fork() == 0) { //child process
                close(listenfd); /* child does not need listening socket */
                executeCmd(connectfd, cmd);
                close(connectfd);
                exit(0);
            }
            close(connectfd); /* parent closes connected socket */
        }
        int status;
        waitpid(-1, &status, WNOHANG);  //wait all child
    }
    close(listenfd);
}
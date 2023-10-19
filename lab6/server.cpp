#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <queue>
#include <ranges>
#include <set>
#include <string_view>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 32768
#define LISTENQ 1024

#define MAXCLIENT 100

class ChatServer {
    unsigned long long m_counter{ 0 };
    int m_port;
    int m_listenfdCmd;
    int m_listenfdSink;

    int m_maxIndexCmd{ 0 };  /* index into client[] array */
    int m_maxIndexSink{ 0 }; /* index into client[] array */
    pollfd m_clientCmd[MAXCLIENT]{};
    pollfd m_clientSink[MAXCLIENT]{};
    int m_sinkClientCount{ 0 };

    unsigned long m_lastTimePoint{ 0 };

    unsigned long getTimeMicrosecond() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return 1000000 * tv.tv_sec + tv.tv_usec;
    }

    void setLastTimePoint() {
        m_lastTimePoint = getTimeMicrosecond();
    }

  public:
    ChatServer(int port)
        : m_listenfdCmd{ socket(AF_INET, SOCK_STREAM, 0) },
          m_listenfdSink{ socket(AF_INET, SOCK_STREAM, 0) },
          m_port{ port } {
        std::ranges::for_each(m_clientCmd, [](pollfd& client) { client.fd = -1;
            client.events = POLLRDNORM | POLLHUP | POLLERR; });
        std::ranges::for_each(m_clientSink, [](pollfd& client) { client.fd = -1;
            client.events = POLLRDNORM | POLLHUP | POLLERR; });
    };

    // start server
    void run() {
        // 9998 cmd
        sockaddr_in servaddrCmd{};
        servaddrCmd.sin_family = AF_INET;
        servaddrCmd.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddrCmd.sin_port = htons(m_port);

        bind(m_listenfdCmd, (sockaddr*)&servaddrCmd, sizeof(servaddrCmd));

        listen(m_listenfdCmd, LISTENQ);

        m_clientCmd[m_listenfdCmd].fd = m_listenfdCmd;
        m_clientCmd[m_listenfdCmd].events = POLLRDNORM;
        if (m_listenfdCmd > m_maxIndexCmd)
            m_maxIndexCmd = m_listenfdCmd;

        // 9999 sink
        sockaddr_in servaddrSink{};
        servaddrSink.sin_family = AF_INET;
        servaddrSink.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddrSink.sin_port = htons(m_port + 1);

        bind(m_listenfdSink, (sockaddr*)&servaddrSink, sizeof(servaddrSink));

        listen(m_listenfdSink, LISTENQ);

        m_clientSink[m_listenfdSink].fd = m_listenfdSink;
        m_clientSink[m_listenfdSink].events = POLLRDNORM;
        if (m_listenfdSink > m_maxIndexSink)
            m_maxIndexSink = m_listenfdSink;
    }

    void clientCmdConnect() {
        sockaddr_in clientAddr;
        socklen_t clientLength{ sizeof(clientAddr) };
        int connfd{ accept(m_listenfdCmd, (sockaddr*)&clientAddr, &clientLength) };

        std::string ip{ inet_ntoa(clientAddr.sin_addr) };
        int port{ ntohs(clientAddr.sin_port) };
        // printf("* cmd client connected from %s:%d\n", ip.c_str(), port);

        if (m_clientCmd[connfd].fd < 0) {
            m_clientCmd[connfd].fd = connfd; /* save descriptor */

            if (connfd > m_maxIndexCmd)
                m_maxIndexCmd = connfd; /* max index in client[] array */

        } else if (connfd > MAXCLIENT)
            fprintf(stderr, "too many clients!\n");
        else // if reach here, means a file descriptor is used twice
            fprintf(stderr, "shold not reach here!\n");
    }

    void clientSinkConnect() {
        sockaddr_in clientAddr;
        socklen_t clientLength{ sizeof(clientAddr) };
        int connfd{ accept(m_listenfdSink, (sockaddr*)&clientAddr, &clientLength) };

        std::string ip{ inet_ntoa(clientAddr.sin_addr) };
        int port{ ntohs(clientAddr.sin_port) };
        // printf("* sink client connected from %s:%d\n", ip.c_str(), port);

        if (m_clientSink[connfd].fd < 0) {
            m_clientSink[connfd].fd = connfd; /* save descriptor */
            m_sinkClientCount++;

            if (connfd > m_maxIndexSink)
                m_maxIndexSink = connfd; /* max index in client[] array */

        } else if (connfd > MAXCLIENT)
            fprintf(stderr, "too many clients!\n");
        else // if reach here, means a file descriptor is used twice
            fprintf(stderr, "shold not reach here!\n");
    }

    void clientCmdDisconnect(const int client) {
        close(client);
        m_clientCmd[client].fd = -1;

        // printf("* cmd client disconnected.\n");
    }

    void clientSinkDisconnect(const int client) {
        close(client);
        m_clientSink[client].fd = -1;
        m_sinkClientCount--;

        // printf("* sink client disconnected.\n");
    }

    void receiveCmdClient(const int client) {
        char buf[MAXLINE + 1]{};
        ssize_t readDataCount{ read(client, buf, MAXLINE) };
        if (readDataCount == 0) {
            // connection closed by client
            clientCmdDisconnect(client);
        } else {
            buf[readDataCount] = '\0';
            std::string_view input{ buf };

            char output[128];
            if (input.starts_with("/reset\n")) {
                sprintf(output, "%ld RESET %lld\n", getTimeMicrosecond(), m_counter);
                m_counter = 0;
                setLastTimePoint();
            } else if (input.starts_with("/ping\n")) {
                sprintf(output, "%ld PONG\n", getTimeMicrosecond());
            } else if (input.starts_with("/report\n")) {
                unsigned long currentTime{ getTimeMicrosecond() };
                float elapsed{ (currentTime - m_lastTimePoint) / 1000000.f };
                float flow{ 8.f * m_counter / 1000000.f / elapsed };
                sprintf(output, "%ld REPORT %lld %fs %.4fMbps\n", currentTime, m_counter, elapsed, flow);
            } else if (input.starts_with("/clients\n")) {
                sprintf(output, "%ld CLIENTS %d\n", getTimeMicrosecond(), m_sinkClientCount);
            }
            send(client, output, strlen(output), MSG_NOSIGNAL);
        }
    }

    void receiveSinkClient(const int client) {
        char buf[MAXLINE + 1]{};
        ssize_t readDataCount{ read(client, buf, MAXLINE) };
        if (readDataCount == 0) {
            // connection closed by client
            clientSinkDisconnect(client);
        } else {
            m_counter += readDataCount;
        }
    }

    void loopCmd() {
        int nreadyCmd{ poll(m_clientCmd, m_maxIndexCmd + 1, 0) };

        if (m_clientCmd[m_listenfdCmd].revents & POLLRDNORM) { // check if new connection
            clientCmdConnect();
            --nreadyCmd;
        }

        if (nreadyCmd <= 0) // nothing available
            return;

        for (int client{ 0 }; client < MAXCLIENT; client++) {
            if (m_clientCmd[client].fd < 0)
                continue;

            if (m_clientCmd[client].revents & (POLLRDNORM | POLLHUP | POLLERR)) { // if receive data
                receiveCmdClient(client);
                if (--nreadyCmd <= 0)
                    return; /* no more readable descriptors */
            }
        }
    }

    void loopSink() {
        // client sink
        int nreadySink{ poll(m_clientSink, m_maxIndexSink + 1, 0) };

        if (m_clientSink[m_listenfdSink].revents & POLLRDNORM) { // check if new connection
            clientSinkConnect();
            --nreadySink;
        }

        if (nreadySink <= 0) // nothing available
            return;

        for (int client{ 0 }; client < MAXCLIENT; client++) {
            if (m_clientSink[client].fd < 0)
                continue;

            if (m_clientSink[client].revents & (POLLRDNORM | POLLHUP | POLLERR)) { // if receive data
                receiveSinkClient(client);
                if (--nreadySink <= 0)
                    return; /* no more readable descriptors */
            }
        }
    }

    void loop() {
        loopCmd();
        loopSink();
    }
};

int main(int argc, char** argv) {
    srand(time(NULL));
    ChatServer chatServer{ std::stoi(argv[1]) };

    chatServer.run();

    while (1)
        chatServer.loop();
}

#include "Core.h"

using namespace ChatServer;

void Core::connect() {
    sockaddr_in clientAddr;
    socklen_t clientLength{ sizeof(clientAddr) };
    int connfd{ accept(m_listenfd, (sockaddr*)&clientAddr, &clientLength) };

    std::string ip{ inet_ntoa(clientAddr.sin_addr) };
    int port{ ntohs(clientAddr.sin_port) };
    serverLog(LogType::Self, "connected", ip, port);
    // printf("* client connected from %s:%d\n", ip.c_str(), port);

    if (m_client[connfd].fd < 0) {
        m_client[connfd].fd = connfd; /* save descriptor */

        manager.addUser(connfd, ip, port);

        if (connfd > m_maxIndex)
            m_maxIndex = connfd; /* max index in client[] array */

    } else if (connfd > FD_SETSIZE)
        fprintf(stderr, "too many clients!\n");
    else // if reach here, means a file descriptor is used twice
        fprintf(stderr, "shold not reach here!\n");
}

void Core::disconnect(const int client) {
    // already disconnected
    if (m_client[client].fd == -1)
        return;

    manager.removeUser(client);

    m_client[client].fd = -1;
    close(client);
}

void Core::kill(const int client) {
    manager.removeUser(client);

    m_client[client].fd = -1;
    shutdown(client, SHUT_RD);
}

void Core::quit(const int client, const Command& command) {
    // do nothing
}

void Core::receiveClient(const int client) {
    char buf[s_maxOutputLength + 1]{};
    ssize_t readDataCount{ read(client, buf, s_maxOutputLength) };
    if (readDataCount == 0) {
        // connection closed by client
        disconnect(client);
    } else {
        buf[readDataCount] = '\0';

        std::string_view input{ buf };
        if (input.back() == '\n' && !input.ends_with("\r\n"))
            input.remove_suffix(1);

                for (std::string_view spliter{ "\r\n" }; const auto inputLine : std::views::split(input, spliter)) {
            if (inputLine.empty())
                continue;
            std::string_view line{ inputLine.begin(), inputLine.end() };
            Command command{ line };

            try {
                manager.commandHandle(client, command);
            } catch (ConnectionEvent event) {
                if (event.code == ConnectionEvent::Kill)
                    kill(client);
                else if (event.code == ConnectionEvent::Disconnect)
                    disconnect(client);
            }
        }
    }
}

Core::Core()
    : m_listenfd{ socket(AF_INET, SOCK_STREAM, 0) } {
    std::ranges::for_each(m_client, [](pollfd& client) { client.fd = -1;
            client.events = POLLRDNORM | POLLHUP | POLLERR; });
};

void Core::run(const int port) {
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    bind(m_listenfd, (sockaddr*)&servaddr, sizeof(servaddr));

    listen(m_listenfd, s_listenQueueSize);

    m_client[m_listenfd].fd = m_listenfd;
    m_client[m_listenfd].events = POLLRDNORM;
    if (m_listenfd > m_maxIndex)
        m_maxIndex = m_listenfd;
}

void Core::loop() {
    int nready{ poll(m_client, m_maxIndex + 1, -1) };

    if (m_client[m_listenfd].revents & POLLRDNORM) { // check if new connection
        connect();
        --nready;
    }

    if (nready <= 0) // nothing available
        return;

    for (int client{ 0 }; client < FD_SETSIZE; client++) {
        if (client < 0)
            continue;

        if (m_client[client].revents & (POLLRDNORM | POLLHUP | POLLERR)) { // if receive data
            receiveClient(client);
            if (--nready <= 0)
                return; /* no more readable descriptors */
        }
    }
}

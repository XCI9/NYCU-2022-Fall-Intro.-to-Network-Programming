#pragma once
#include "Command.h"
#include "CommandHandler.h"
#include "ServerConfig.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <ranges>
#include <string_view>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace ChatServer {

class Core {
    CommandHandler manager;

    int m_listenfd;

    int m_maxIndex{ 0 }; /* index into client[] array */
    pollfd m_client[FD_SETSIZE]{};

    void sendErrorMsg(const int client, Response errorCode, const std::string& msg);

    void connect();

    void disconnect(const int client);

    void receiveClient(const int client);

    void quit(const int client, const Command& command);

    void kill(const int client);

  public:
    Core();

    void run(const int port);

    void loop();
};
} // namespace ChatServer
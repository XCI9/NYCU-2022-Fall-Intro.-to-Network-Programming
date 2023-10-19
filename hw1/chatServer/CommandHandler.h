#pragma once
#include "Channel.h"
#include "Command.h"
#include "EntityManager.h"

#include "User.h"
#include <cstdio>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <string>

namespace ChatServer {

class CommandHandler {
    EntityManager m_entity;

    std::map<std::string, std::function<void(const int, const Command&)>> m_functionLookup;

    void joinChannel(const int client, const Command& command);

    void leaveChannel(const int client, const Command& command);

    void listChannel(const int client, const Command& command);

    void listChannelUser(const int client, const Command& command);

    void setChannelTopic(const int client, const Command& command);

    void ping(const int client, const Command& command);

    void setNickName(const int fd, const Command& command);

    void sendPrivateMsg(const int sender, const Command& command);

    void listAllUser(const int receiver, /*[[maybe_usused]]*/ const Command& command);

    void userInit(const int client, const Command& command);

    void errorHandle(const int client, const Error& error, const Command& command);

    void functionLookupInit();

    void quit(const int client, const Command& command);

  public:
    CommandHandler();

    void removeUser(const int client);

    void addUser(const int client, const std::string_view ip, const int port);

    void commandHandle(const int client, const Command& command);
};
} // namespace ChatServer

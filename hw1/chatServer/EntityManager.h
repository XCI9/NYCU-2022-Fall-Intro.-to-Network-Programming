#pragma once
#include "Channel.h"
#include "Command.h"

#include "User.h"
#include <cstdio>
#include <list>
#include <map>
#include <string>

namespace ChatServer {

class EntityManager {
    int generateChannelID() const;
    std::list<User> m_userStorage;
    std::list<Channel> m_channelStorage;

    std::map<std::string_view, int> m_entityName; // reference is valid if fd is used
    std::map<int, Entity*> m_entity;

  public:
    Channel* getChannelByName(std::string_view name);

    Channel* getChannelById(const int id);

    User* getUserByName(std::string_view name);

    User* getUserById(const int id);

    Entity* getEntityByName(std::string_view name);

    Entity* getEntityById(const int id);

    Entity* operator[](const int id);

    bool isNameExist(std::string_view name) const;

    bool isIdExist(const int id) const;

    void setNickName(const int id, std::string_view nick);

    void createChannelIfNotExist(std::string_view name);

    void changeNickName(const int id, std::string_view nick);

    void setUserInfo(const int id, std::string_view username, std::string_view hostname,
                     std::string_view servername, std::string_view realname);

    void removeUser(User* user);

    void addUser(const int client, const std::string_view ip, const int port);

    std::size_t size() const;

    const std::list<User>& getUserList() const;

    const std::list<Channel>& getChannelList() const;

    bool isUserRegister(const int client);
};
} // namespace ChatServer

#pragma once

#include "Entity.h"
#include "FastString.h"
#include "ServerConfig.h"
#include <list>
#include <set>
#include <sys/socket.h>

namespace ChatServer {
class User : public Entity {
    std::string m_username;
    std::string m_hostname;
    std::string m_servername;
    std::string m_realname;
    std::string m_ip;
    int m_port;

    std::set<int> m_joinedChannel;
    std::list<User>::iterator m_storagePos;

    bool m_isUserInfoInit{ false };
    bool m_isNickInit{ false };

  public:
    User(const int fd, std::string_view name);

    std::string& ip();

    const std::string& ip() const;

    void initName(std::string_view name);

    int& port();

    std::list<User>::iterator& storage();

    void joinChannel(int channelId);

    void leaveChannel(const int channelId);

    const std::set<int>& getJoinedChannelList();

    void sendMsg(std::string_view msg, const int sender = -1) const override;

    void sendMsg(const Response response, std::span<std::string_view> stringList) const override;

    void sendClientMsg(std::string_view senderName, std::string_view msg) const override;

    void echoCmd(std::string_view msg);

    void setUserInfo(std::string_view username, std::string_view hostname,
                     std::string_view servername, std::string_view realname);

    bool isRegistered() const;
};

struct Compare {
    bool operator()(const User* user1, const User* user2) const {
        return user1 > user2;
    }
};
} // namespace ChatServer
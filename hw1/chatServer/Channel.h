#pragma once

#include "Entity.h"
#include "Error.h"
#include "Response.h"
#include "User.h"
#include <list>
#include <set>

namespace ChatServer {
class Channel : public Entity {
    std::set<User*, Compare> m_user;
    std::string m_topic;
    std::list<Channel>::iterator m_storagePos;

  public:
    Channel(const int fd, std::string_view name);

    void sendMsg(std::string_view msg, const int sender) const override;

    void sendMsg(const Response response, std::span<std::string_view> stringList) const override;

    void sendClientMsg(std::string_view senderName, std::string_view msg) const override;

    std::list<Channel>::iterator& storage();

    const std::set<User*, Compare>& getUserList() const;

    int getClientCount() const;

    std::string_view getTopic() const;

    void setTopic(std::string_view topic);

    void addUser(User* user);

    void removeUser(User* user);
};
} // namespace ChatServer
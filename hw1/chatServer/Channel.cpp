#include "Channel.h"

using namespace ChatServer;

Channel::Channel(const int fd, std::string_view name) : Entity{ fd, name } {}

void Channel::sendMsg(std::string_view msg, const int sender) const {
    for (const auto client : m_user) {
        if (client->getId() == sender)
            continue;
        client->sendMsg(msg);
    }
}

void Channel::sendMsg(const Response response, std::span<std::string_view> stringList) const {
    if (m_user.empty())
        throw Error{ Response::ERR_NORECIPIENT };
    for (const auto client : m_user) {
        client->sendMsg(response, stringList);
    }
}

void Channel::sendClientMsg(std::string_view senderName, std::string_view msg) const {
    sendMsg(FastString::concatenate(":", senderName, " ", msg), -1);
}

std::list<Channel>::iterator& Channel::storage() {
    return m_storagePos;
}
const std::set<User*, Compare>& Channel::getUserList() const {
    return m_user;
}

int Channel::getClientCount() const {
    return m_user.size();
}

std::string_view Channel::getTopic() const {
    return m_topic;
}

void Channel::setTopic(std::string_view topic) {
    m_topic = topic;
}

void Channel::addUser(User* user) {
    m_user.insert(user);
}

void Channel::removeUser(User* user) {
    if (!m_user.contains(user))
        throw Error{ Response::ERR_NOTONCHANNEL, m_name };
    m_user.erase(user);
}

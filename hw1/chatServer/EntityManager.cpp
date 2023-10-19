#include "EntityManager.h"

using namespace ChatServer;

void EntityManager::createChannelIfNotExist(std::string_view name) {
    if (!m_entityName.contains(name)) {
        Channel newChannel{ generateChannelID(), name };
        m_channelStorage.push_back(newChannel);
        m_channelStorage.back().storage() = std::prev(m_channelStorage.end());
        Entity* e{ dynamic_cast<Entity*>(&m_channelStorage.back()) };
        m_entityName[e->getName()] = e->getId();
        m_entity[e->getId()] = e;
    }
}

Channel* EntityManager::getChannelByName(std::string_view name) {
    if (!m_entityName.contains(name))
        throw Error{ Response::ERR_NOSUCHCHANNEL };
    return dynamic_cast<Channel*>(m_entity[m_entityName[name]]);
}

Channel* EntityManager::getChannelById(const int id) {
    return dynamic_cast<Channel*>(m_entity[id]);
}

User* EntityManager::getUserByName(std::string_view name) {
    if (!m_entityName.contains(name))
        throw Error{ Response::ERR_NOSUCHNICK };
    return dynamic_cast<User*>(m_entity[m_entityName[name]]);
}

User* EntityManager::getUserById(const int id) {
    return &*(dynamic_cast<User*>(m_entity[id]));
}

Entity* EntityManager::getEntityByName(std::string_view name) {
    if (!m_entityName.contains(name))
        throw Error{ Response::ERR_NOSUCHNICK };
    return m_entity[m_entityName[name]];
}

Entity* EntityManager::getEntityById(const int id) {
    // if (id == m_tempUser.id())
    //     return m_tempUser;
    return &*m_entity[id];
}

Entity* EntityManager::operator[](const int id) {
    return getEntityById(id);
}

bool EntityManager::isNameExist(std::string_view name) const {
    return m_entityName.contains(name);
}

bool EntityManager::isIdExist(const int id) const {
    return m_entity.contains(id);
}

void EntityManager::changeNickName(const int id, std::string_view nick) {
    User* user{ getUserById(id) };

    // need to remove from entityName first and add back after change nick
    m_entityName.erase(user->getName());
    user->setName(nick);
    m_entityName[user->getName()] = user->getId();
}

void EntityManager::setNickName(const int id, std::string_view nick) {
    if (isNameExist(nick)) {
        throw Error{ Response::ERR_NICKCOLLISION, nick };
    }

    if (User * user{ getUserById(id) }; !user->isRegistered()) { // init nick name
        user->initName(nick);
        if (user->isRegistered())
            m_entityName[user->getName()] = user->getId();
    } else { // change nick name
        changeNickName(id, nick);
    }
}

void EntityManager::setUserInfo(const int id, std::string_view username, std::string_view hostname,
                                std::string_view servername, std::string_view realname) {
    User* entity{ getUserById(id) };

    entity->setUserInfo(username, hostname, servername, realname);

    if (entity->isRegistered())
        m_entityName[entity->getName()] = entity->getId();
}

void EntityManager::removeUser(User* user) {
    //   remove from entity set
    auto storePos{ user->storage() };
    if (storePos->isRegistered())
        m_entityName.erase(m_entity[user->id()]->getName());
    m_entity.erase(user->id());
    m_userStorage.erase(storePos);
}

void EntityManager::addUser(const int client, const std::string_view ip, const int port) {
    User newUser{ client, "" };

    newUser.id() = client;
    newUser.ip() = ip;
    newUser.port() = port;

    m_userStorage.push_back(newUser);
    m_userStorage.back().storage() = std::prev(m_userStorage.end());

    m_entity[client] = &*m_userStorage.back().storage();
}

int EntityManager::generateChannelID() const {
    static int id{ -1 };
    return id--;
}

std::size_t EntityManager::size() const {
    return m_entity.size();
}

const std::list<User>& EntityManager::getUserList() const {
    return m_userStorage;
}

const std::list<Channel>& EntityManager::getChannelList() const {
    return m_channelStorage;
}

bool EntityManager::isUserRegister(const int client) {
    return getUserById(client)->isRegistered();
}
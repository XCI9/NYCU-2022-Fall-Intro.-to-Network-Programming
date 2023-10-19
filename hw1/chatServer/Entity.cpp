#include "Entity.h"

using namespace ChatServer;

Entity::Entity(const int id, std::string_view name) : m_name{ name },
                                                      m_id{ id } {}

int Entity::getId() const {
    return m_id;
}

int& Entity::id() {
    return m_id;
}

const std::string& Entity::getName() const {
    return m_name;
}

void Entity::setName(std::string_view name) {
    m_name = name;
}

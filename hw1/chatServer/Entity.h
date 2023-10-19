#pragma once
#include "FastString.h"
#include "Response.h"
#include <span>
#include <string>
#include <string_view>

namespace ChatServer {

class Entity {
  protected:
    std::string m_name;
    int m_id;

  public:
    Entity(const int id = 0, std::string_view name = "");

    int getId() const;

    int& id();

    virtual void sendMsg(std::string_view msg, const int sender = -1) const = 0;

    virtual void sendMsg(const Response response, std::span<std::string_view> stringList) const = 0;

    template <typename... Strings>
    void sendServerMsg(const Response response, Strings&&... msg) const;

    virtual void sendClientMsg(std::string_view senderName, std::string_view msg) const = 0;

    const std::string& getName() const;

    void setName(std::string_view name);
};
} // namespace ChatServer

#include "Entity.tpp"

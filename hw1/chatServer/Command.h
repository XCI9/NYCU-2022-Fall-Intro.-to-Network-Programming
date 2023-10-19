#pragma once

#include "Error.h"
#include "Response.h"
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace ChatServer {
class Command {
    std::string m_fullCommand;
    std::vector<std::string_view> m_commandList;

    void generateCommandList();

  public:
    Command(std::string_view fullCommand);

    std::string_view getFullCommand() const;

    const std::vector<std::string_view>& getCommandList();

    std::size_t size() const;

    std::string_view operator[](const std::size_t i) const;

    operator std::string_view() const;
};
} // namespace ChatServer
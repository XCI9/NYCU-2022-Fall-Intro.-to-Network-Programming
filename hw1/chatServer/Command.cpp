#include "Command.h"

using namespace ChatServer;
void Command::generateCommandList() {
    std::string_view input{ m_fullCommand };

    std::size_t lastSpacePos{ 0 };
    std::size_t pos{ 0 };
    while (pos < m_fullCommand.size()) {
        if (input[pos] == ' ') {
            m_commandList.push_back(input.substr(lastSpacePos, pos - lastSpacePos));
            lastSpacePos = pos + 1;

            if (input[pos + 1] == ':') { // last parameter
                pos += 2;
                break;
            }
        }
        pos++;
    }
    if (pos < input.size()) {
        m_commandList.push_back(input.substr(pos));
    } else {
        m_commandList.push_back(input.substr(lastSpacePos));
    }
}

Command::Command(std::string_view fullCommand) : m_fullCommand{ fullCommand } {
    generateCommandList();
};

std::string_view Command::getFullCommand() const {
    return m_fullCommand;
}

const std::vector<std::string_view>& Command::getCommandList() {
    return m_commandList;
}

std::size_t Command::size() const {
    return m_commandList.size();
}

std::string_view Command::operator[](const std::size_t i) const {
    if (m_commandList.size() <= i)
        throw Error{ Response::ERR_NEEDMOREPARAMS };

    return m_commandList[i];
}

Command::operator std::string_view() const {
    return m_fullCommand;
}

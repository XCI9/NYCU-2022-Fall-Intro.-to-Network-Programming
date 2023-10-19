#include "CommandHandler.h"

using namespace ChatServer;

void CommandHandler::joinChannel(const int client, const Command& command) {
    std::string_view channelName{ command[1] };

    m_entity.createChannelIfNotExist(channelName);

    Channel* channel{ m_entity.getChannelByName(channelName) };
    User* user{ m_entity.getUserById(client) };
    channel->addUser(user);

    user->joinChannel(channel->getId());
    // user->echoCmd(command);
    channel->sendClientMsg(user->getName(), command);

    // send channel topic
    if (channel->getTopic().length() == 0)
        m_entity[client]->sendServerMsg(Response::RPL_NOTOPIC, channelName, "No topic is set");
    else
        m_entity[client]->sendServerMsg(Response::RPL_TOPIC, channelName, channel->getTopic());

    // send nameList
    std::string nameList;
    for (const auto user : channel->getUserList())
        nameList += user->getName() + " ";
    // remove last " "
    if (channel->getClientCount() > 0)
        nameList.pop_back();
    m_entity[client]->sendServerMsg(Response::RPL_NAMREPLY, channelName, nameList);
    m_entity[client]->sendServerMsg(Response::RPL_ENDOFNAMES, channelName, "End of /NAMES list");
}

void CommandHandler::leaveChannel(const int client, const Command& command) {
    std::string_view channelName{ command[1] };

    Channel* channel{ m_entity.getChannelByName(channelName) };

    User* user{ m_entity.getUserById(client) };
    channel->removeUser(user);
    user->leaveChannel(channel->getId());
    // user->echoCmd(command);
    // broadcast
    channel->sendClientMsg(user->getName(), command);
}

void CommandHandler::setNickName(const int fd, const Command& command) {
    if (command.size() < 2)
        throw Error{ Response::ERR_NONICKNAMEGIVEN };

    std::string_view nick{ command[1] };

    m_entity.setNickName(fd, nick);
}

void CommandHandler::userInit(const int client, const Command& command) {
    std::string_view username{ command[1] };
    std::string_view hostname{ command[2] };
    std::string_view servername{ command[3] };
    std::string_view realname{ command[4] };

    m_entity.setUserInfo(client, username, hostname, servername, realname);

    // welcome msg
    m_entity[client]->sendServerMsg(Response::RPL_WELCOME, "Welcome to the Internet Relay Network");
    m_entity[client]->sendServerMsg(Response::RPL_LUSERCLIENT,
                                    FastString::concatenate("There are ", std::to_string(m_entity.size()),
                                                            " users and 0 invisible on 1 server"));
    m_entity[client]->sendServerMsg(Response::RPL_MOTDSTART, "Start msg of the day.");
    // Craawford2 ascii art
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(    __  __ __   ____  ______       _____   ___  ____  __ __    ___  ____  )");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(   /  ]|  |  | /    ||      |     / ___/  /  _]|    \|  |  |  /  _]|    \ )");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(  /  / |  |  ||  o  ||      |    (   \_  /  [_ |  D  )  |  | /  [_ |  D  ))");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"( /  /  |  _  ||     ||_|  |_|     \__  ||    _]|    /|  |  ||    _]|    / )");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(/   \_ |  |  ||  _  |  |  |       /  \ ||   [_ |    \|  :  ||   [_ |    \ )");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(\     ||  |  ||  |  |  |  |       \    ||     ||  .  \\   / |     ||  .  \)");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"( \____||__|__||__|__|  |__|        \___||_____||__|\_| \_/  |_____||__|\_|)");
    m_entity[client]->sendServerMsg(Response::RPL_MOTD, R"(                                                                          )");
    m_entity[client]->sendServerMsg(Response::RPL_ENDOFMOTD, "End msg of the day.");
}

void CommandHandler::listChannel(const int client, const Command& command) {

    m_entity[client]->sendServerMsg(Response::RPL_LISTSTART, "Channel", "Users  Name");

    if (command.size() < 2) { // no specify channel, list all
        for (auto& channel : m_entity.getChannelList()) {
            m_entity[client]->sendServerMsg(Response::RPL_LIST, channel.getName(),
                                            std::to_string(channel.getClientCount()), channel.getTopic());
        }
    } else {
        std::vector<std::string> specifyChannel;
        for (std::string_view spliter{ "," }, specifyChannelString{ command[1] };
             const auto& channel : std::views::split(specifyChannelString, spliter)) {
            std::string newChannel{ channel.begin(), channel.end() };
            specifyChannel.push_back(newChannel);
        }

        for (const auto& channelName : specifyChannel) {
            Channel* channel{ m_entity.getChannelByName(channelName) };
            m_entity[client]->sendServerMsg(Response::RPL_LIST,
                                            channelName, std::to_string(channel->getClientCount()), channel->getTopic());
        }
    }

    m_entity[client]->sendServerMsg(Response::RPL_LISTEND, "End of /LIST");
}

void CommandHandler::setChannelTopic(const int client, const Command& command) {
    std::string_view channelName{ command[1] };

    Channel* channel;

    try {
        channel = m_entity.getChannelByName(channelName);
    } catch (Error error) {
        throw Error{ Response::ERR_NOTONCHANNEL };
    }

    if (User * user{ m_entity.getUserById(client) }; !channel->getUserList().contains(user))
        throw Error{ Response::ERR_NOTONCHANNEL, channel->getName() };

    // command contain topic string
    if (command.size() == 3) {
        std::string_view newTopic{ command[2] };
        channel->setTopic(newTopic);
    }
    if (channel->getTopic().length() == 0)
        m_entity[client]->sendServerMsg(Response::RPL_NOTOPIC, channelName, "No topic is set");
    else
        m_entity[client]->sendServerMsg(Response::RPL_TOPIC, channelName, channel->getTopic());
}

void CommandHandler::listChannelUser(const int receiver, const Command& command) {
    std::string_view channelName{ command[1] };

    Channel* channel{ m_entity.getChannelByName(channelName) };

    m_entity[receiver]->sendServerMsg(Response::RPL_TOPIC, channelName, channel->getTopic());

    std::string nameList{};
    for (const auto client : channel->getUserList()) {
        nameList += m_entity[client->getId()]->getName();
    }
    if (nameList.size() != 0 && nameList.back() == ' ')
        nameList.pop_back();

    m_entity[receiver]->sendServerMsg(Response::RPL_NAMREPLY, channelName, nameList);
    m_entity[receiver]->sendServerMsg(Response::RPL_ENDOFNAMES, channelName, "End of /NAMES list");
}

void CommandHandler::listAllUser(const int receiver, const Command& command) {
    char buffer[128];
    sprintf(buffer, "%-8s %-9s %-8s", "UserID", "Terminal", "Host");
    m_entity[receiver]->sendServerMsg(Response::RPL_USERSSTART, buffer);
    for (auto& user : m_entity.getUserList()) {
        sprintf(buffer, "%-8s %-9s %-8s", user.getName().c_str(), "-", user.ip().c_str());
        m_entity[receiver]->sendServerMsg(Response::RPL_USERS, buffer);
    }
    m_entity[receiver]->sendServerMsg(Response::RPL_ENDOFUSERS, "End of users");
}

void CommandHandler::sendPrivateMsg(const int sender, const Command& command) {
    if (command.size() < 2)
        throw Error{ Response::ERR_NORECIPIENT };
    if (command.size() < 3)
        throw Error{ Response::ERR_NOTEXTTOSEND };
    std::string_view receiverNick{ command[1] };

    Entity* entity{ m_entity.getEntityByName(receiverNick) };

    // entity->sendMsg(FastString::concatenate(":", m_entity[sender]->getName(), " ", command.getFullCommand()), sender);
    entity->sendClientMsg(m_entity[sender]->getName(), command);
}

void CommandHandler::ping(const int client, const Command& command) {
    m_entity.getUserById(client)->sendMsg(FastString::concatenate("PONG ", command[1]));
}

void CommandHandler::removeUser(const int client) {
    User* user{ m_entity.getUserById(client) };

    serverLog(LogType::Self, "disconnected", user->ip(), user->port());
    // printf("* client \"%s\", ip %s:%d disconnected.\n", user->getName().c_str(), user->ip().c_str(), user->port());
    //  leave all joined channel
    for (const auto ch : user->getJoinedChannelList()) {
        Channel* channel{ m_entity.getChannelById(ch) };
        channel->removeUser(user);
    }

    m_entity.removeUser(user);
}

void CommandHandler::quit(const int client, const Command& command) {
    throw ConnectionEvent{ ConnectionEvent::Disconnect };
    // do nothing
}

void CommandHandler::addUser(const int client, const std::string_view ip, const int port) {
    m_entity.addUser(client, ip, port);
}

void CommandHandler::errorHandle(const int client, const Error& error, const Command& command) {
    switch (error.errorCode) {
    case Response::ERR_NOSUCHNICK: //"<nickname> :No such nick/channel"
        m_entity[client]->sendServerMsg(error.errorCode, error.parameter, "No such nick/channel");
        break;
    case Response::ERR_NOSUCHCHANNEL: //"<channel name> :No such channel"
        m_entity[client]->sendServerMsg(error.errorCode, error.parameter, "No such channel");
        break;
    case Response::ERR_NORECIPIENT: //":No recipient given (<command>)"
        m_entity[client]->sendServerMsg(error.errorCode, FastString::concatenate("No recipient given (", command, ")"));
        break;
    case Response::ERR_NICKCOLLISION: //"<nick> :Nickname collision KILL"
        m_entity[client]->sendServerMsg(error.errorCode, error.parameter, "Nickname collision KILL");
        throw ConnectionEvent{ ConnectionEvent::Kill };
        // kill(client); // kill
        break;
    case Response::ERR_NOTONCHANNEL: //"<channel> :You're not on that channel"
        m_entity[client]->sendServerMsg(error.errorCode, error.parameter, "You're not on that channel");
        break;
    case Response::ERR_NOTREGISTERED: //":You have not registered"
        m_entity[client]->sendServerMsg(error.errorCode, "You have not registered");
        break;
    case Response::ERR_NEEDMOREPARAMS: //"<command> :Not enough parameters"
        m_entity[client]->sendServerMsg(error.errorCode, command, "Not enough parameters");
        break;

    case Response::ERR_NONICKNAMEGIVEN: //":No nickname given"
        m_entity[client]->sendServerMsg(error.errorCode, "No nickname given");
        break;
    case Response::ERR_NOTEXTTOSEND: //":No text to send"
        m_entity[client]->sendServerMsg(error.errorCode, "No text to send");
        break;
    case Response::ERR_UNKNOWNCOMMAND: //"<command> :Unknown command"
        m_entity[client]->sendServerMsg(error.errorCode, command, "Unknown command");
        break;
    default:
        fprintf(stderr, "Unknown error occured!\n");
    }
}

void CommandHandler::functionLookupInit() {
    using std::placeholders::_1, std::placeholders::_2;
    m_functionLookup["NICK"] = std::bind(&CommandHandler::setNickName, this, _1, _2);
    m_functionLookup["USER"] = std::bind(&CommandHandler::userInit, this, _1, _2);
    m_functionLookup["PING"] = std::bind(&CommandHandler::ping, this, _1, _2);
    m_functionLookup["LIST"] = std::bind(&CommandHandler::listChannel, this, _1, _2);
    m_functionLookup["JOIN"] = std::bind(&CommandHandler::joinChannel, this, _1, _2);
    m_functionLookup["TOPIC"] = std::bind(&CommandHandler::setChannelTopic, this, _1, _2);
    m_functionLookup["NAMES"] = std::bind(&CommandHandler::listChannelUser, this, _1, _2);
    m_functionLookup["PART"] = std::bind(&CommandHandler::leaveChannel, this, _1, _2);
    m_functionLookup["USERS"] = std::bind(&CommandHandler::listAllUser, this, _1, _2);
    m_functionLookup["PRIVMSG"] = std::bind(&CommandHandler::sendPrivateMsg, this, _1, _2);
    m_functionLookup["QUIT"] = std::bind(&CommandHandler::quit, this, _1, _2);
}

void CommandHandler::commandHandle(const int client, const Command& command) {
    try {
        serverLog(LogType::Receive, command, m_entity.getUserById(client)->ip(), m_entity.getUserById(client)->port());
        std::string commandName{ command[0] };
        // if (!m_entity.isUserRegister(client) && commandName != "NICK" && commandName != "USER")
        //     throw Error{ Response::ERR_NOTREGISTERED };

        if (m_functionLookup.contains(commandName))
            m_functionLookup[commandName](client, command);
        else
            throw Error{ Response::ERR_UNKNOWNCOMMAND, "" };
    } catch (Error error) {
        errorHandle(client, error, command);
    }
}

CommandHandler::CommandHandler() {
    functionLookupInit();
}
#include "User.h"

using namespace ChatServer;

User::User(const int fd, std::string_view name) : Entity{ fd, name } {}

std::string& User::ip() { return m_ip; }

const std::string& User::ip() const { return m_ip; }

int& User::port() { return m_port; }

std::list<User>::iterator& User::storage() {
    return m_storagePos;
}

void User::joinChannel(int channelId) {
    m_joinedChannel.insert(channelId);
}

void User::leaveChannel(const int channelId) {
    m_joinedChannel.erase(channelId);
}

const std::set<int>& User::getJoinedChannelList() {
    return m_joinedChannel;
}

void User::sendMsg(std::string_view msg, const int sender) const {
    std::string output{ FastString::concatenate(msg, "\r\n") };
    send(m_id, output.c_str(), output.length(), MSG_NOSIGNAL);
}

void User::sendMsg(const Response response, std::span<std::string_view> stringList) const {
    static constexpr const char* serverPrefix{ ":chatServer" };
    char output[s_maxOutputLength + 1];
    int currentPos{ 0 };

    // server name
    currentPos += sprintf(&output[currentPos], "%s ", serverPrefix);

    // response code
    currentPos += sprintf(&output[currentPos], "%03d", static_cast<int>(response));

    // receiver name
    currentPos += sprintf(&output[currentPos], " %s", m_name.c_str());

    // https://stackoverflow.com/questions/72575514/correct-way-to-printf-a-stdstring-view
    // first n - 1 parameter
    for (const auto& str : stringList.first(stringList.size() - 1))
        currentPos += sprintf(&output[currentPos], " %.*s", static_cast<int>(str.length()), str.data());

    // last parameter, add ':' at begin
    currentPos += sprintf(&output[currentPos], " :%.*s", static_cast<int>(stringList.back().length()), stringList.back().data());

    currentPos += sprintf(&output[currentPos], "\r\n");

    serverLog(LogType::Send, output, m_ip, m_port);
    // printf("%s", output);
    send(m_id, output, currentPos, MSG_NOSIGNAL);
}

void User::sendClientMsg(std::string_view senderName, std::string_view msg) const {
    sendMsg(FastString::concatenate(":", senderName, " ", msg));
}

void User::echoCmd(std::string_view msg) {
    sendMsg(FastString::concatenate(":", m_name, " ", msg));
}

void User::setUserInfo(std::string_view username, std::string_view hostname, std::string_view servername, std::string_view realname) {
    m_username = username;
    m_hostname = hostname;
    m_servername = servername;
    m_realname = realname;
    m_isUserInfoInit = true;
}

bool User::isRegistered() const {
    return m_isNickInit && m_isUserInfoInit;
}

void User::initName(std::string_view name) {
    m_name = name;
    m_isNickInit = true;
}
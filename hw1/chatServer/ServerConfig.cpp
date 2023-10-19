#include "ServerConfig.h"

void ChatServer::serverLog(LogType type, std::string_view msg, std::string_view ip, int port) {
    if (static_cast<int>(type) & s_showedLogType) {
        if (type == LogType::Send)
            printf("[%15.*s:%5d <--] %.*s", static_cast<int>(ip.length()), ip.data(), port, static_cast<int>(msg.length()), msg.data());
        else if (type == LogType::Receive)
            printf("[%15.*s:%5d -->] %.*s", static_cast<int>(ip.length()), ip.data(), port, static_cast<int>(msg.length()), msg.data());
        else
            printf("[%21s ***] client %.*s:%5d %.*s", "", static_cast<int>(ip.length()), ip.data(), port,
                   static_cast<int>(msg.length()), msg.data());
        if (msg.back() != '\n')
            printf("\n");
    }
}
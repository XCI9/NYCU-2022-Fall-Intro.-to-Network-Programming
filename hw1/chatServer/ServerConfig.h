#pragma once
#include "FastString.h"
#include <string_view>

namespace ChatServer {

enum class LogType {
    Self = 1,
    Send = 2,
    Receive = 4
};

inline constexpr int s_maxOutputLength{ 500 };
inline constexpr int s_listenQueueSize{ 1024 };
inline constexpr int s_showedLogType{ static_cast<int>(LogType::Self) | static_cast<int>(LogType::Send) | static_cast<int>(LogType::Receive) };

void serverLog(LogType type, std::string_view msg, std::string_view ip = "", int port = -1);
} // namespace ChatServer
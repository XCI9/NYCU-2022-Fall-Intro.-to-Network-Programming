#pragma once
#include "Response.h"

#include <string>
#include <string_view>

namespace ChatServer {
struct Error {
    Response errorCode;
    std::string parameter;

    Error(Response code, std::string_view sv = "");
};

struct ConnectionEvent {
    enum {
        Kill,
        Disconnect
    } code;
};

} // namespace ChatServer
#include "Error.h"

using namespace ChatServer;

Error::Error(Response code, std::string_view sv) : errorCode{ code },
                                                   parameter{ sv } {}
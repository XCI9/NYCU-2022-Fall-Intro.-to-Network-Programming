#include "Server.h"

#pragma pack(1)

int main(int argc, char* argv[]) {
    // argv: ./dns <port-number> <path/to/the/config/file>
    int port{ atoi(argv[1]) };
    const char* configFilePath{ argv[2] };

    DNSServer::Server server{ port, configFilePath };

    server.checkConnection();
}
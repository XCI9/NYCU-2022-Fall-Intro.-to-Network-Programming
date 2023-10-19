#include "Core.h"

int main(int argc, char* argv[]) {
    if (argc < 2)
        printf("use ./main <port>");
    ChatServer::Core server;
    server.run(std::atoi(argv[1]));

    while (1)
        server.loop();
}
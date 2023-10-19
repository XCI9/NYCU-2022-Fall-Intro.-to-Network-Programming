#include "Server.h"

#pragma pack(1)

using namespace DNSServer;

Server::Server(const int port, const char* configFilePath) : m_database{ configFilePath },
                                                             m_socketFd{ socket(AF_INET, SOCK_DGRAM, 0) } {
    sockaddr_in servaddr{};

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    bind(m_socketFd, (sockaddr*)&servaddr, sizeof(servaddr));
}

void Server::print_hex(const char* s, int n) {
    int printCount{ 0 };
    while (n-- > 0) {
        printCount++;
        printf("%02x ", (unsigned char)*s++);
        if (printCount % 8 == 0)
            printf("\n");
    }
    printf("\n");
}

std::size_t Server::memcpyRecord(char* destination, const std::shared_ptr<Record>& record) {
    char* startPos{ destination };

    uint16_t QType{ htons(static_cast<uint16_t>(record->type())) };
    uint16_t QClass{ htons(static_cast<uint16_t>(record->cls())) };
    uint32_t TTL{ htonl(record->TTL()) };

    std::string rdata{ record->getAsRData() };
    uint16_t dataLength{ htons(static_cast<uint16_t>(rdata.size())) }; // 1 is '\0'

    for (std::string_view splitter{ "." }, lineView{ record->fullName() };
         const auto split : std::views::split(lineView, splitter)) {
        std::string_view dataPart{ split.begin(), split.end() };
        uint8_t length{ static_cast<uint8_t>(dataPart.size()) };
        destination += memcpyN(destination, &length, sizeof(length));
        destination += sprintf(destination, "%.*s", static_cast<int>(dataPart.length()), dataPart.data());
    }
    destination += 1; //'\0'
    destination += memcpyN(destination, &QType, sizeof(QType));
    destination += memcpyN(destination, &QClass, sizeof(QClass));
    destination += memcpyN(destination, &TTL, sizeof(TTL));
    destination += memcpyN(destination, &dataLength, sizeof(dataLength));
    destination += memcpyN(destination, rdata.c_str(), rdata.size());

    return destination - startPos;
}

void Server::checkConnection() {
    sockaddr pcliaddr{};

    socklen_t len;
    char msg[1024]{};
    while (1) {
        len = sizeof(pcliaddr);
        auto n{ recvfrom(m_socketFd, msg, sizeof(msg), 0, &pcliaddr, &len) };
        char* msgPos{ msg };

        Header header;
        msgPos += memcpyN(&header, msgPos, sizeof(header));
        header.ntoh();

        std::vector<std::string_view> labels;
        uint8_t labelLength;
        do {
            msgPos += memcpyN(&labelLength, msgPos, sizeof(labelLength));
            labels.emplace_back(msgPos, msgPos + labelLength);
            msgPos += labelLength;
        } while (labelLength != 0);
        labels.pop_back(); // reomove last null label

        std::string domain{ combine(labels) };

        uint16_t QType;
        uint16_t QClass;
        msgPos += memcpyN(&QType, msgPos, sizeof(QType));
        QType = ntohs(QType);
        msgPos += memcpyN(&QClass, msgPos, sizeof(QClass));
        QClass = ntohs(QClass);

        char* questionEndPos{ msgPos };

        char* remainingField{ msgPos };
        long int remainingFieldLength{ &msg[n] - msgPos };

        std::vector<std::string_view> domainNameVector{ split(domain, ".") };
        std::span<std::string_view> domainNameSpan{ domainNameVector };
        std::span<std::string_view> ipNIPIO;
        bool isNIPIO{ false };
        static std::regex AddressNIPIO{
            R"(([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})\.([0-9a-zA-Z]{1,61}\.)*[0-9a-zA-Z]{1,61})"
        };
        if (std::regex_match(domain, AddressNIPIO)) {
            isNIPIO = true;
            printf("match!\n");
            ipNIPIO = domainNameSpan.first(4);
            domainNameSpan = domainNameSpan.subspan(4);
        }

        if (std::shared_ptr<Domain> zoneData{ m_database().getDomain(domainNameSpan) }; zoneData != nullptr) {
            std::vector<std::shared_ptr<Record>> answers;
            std::vector<std::shared_ptr<Record>> authorities;
            std::vector<std::shared_ptr<Record>> additionals;

            if (!isNIPIO) {
                answers = m_database().getRecord(domainNameSpan, static_cast<Record::Type>(QType),
                                                 static_cast<Record::Class>(QClass));

                for (const std::shared_ptr<Record>& answer : answers) {
                    std::string_view additionalKey{ answer->getAdditionalKey() };

                    auto s{ split(additionalKey, ".") };
                    auto additional{ m_database().getRecord(s) };
                    additionals.insert(additionals.end(), additional.begin(), additional.end());
                }
            } else {
                std::string ip{ combine(ipNIPIO) };

                answers.emplace_back(std::make_shared<ARecord>(domain, "", Record::Class::IN, 1, ip.c_str()));
            }

            if (static_cast<Record::Type>(QType) != Record::Type::NS) {
                if (answers.empty())
                    authorities = zoneData->SOA();
                else
                    authorities = zoneData->NS();
            }

            header.ANCount = answers.size();
            header.ARCount += additionals.size();
            header.NSCount = authorities.size();
            header.setQR(1);
            header.hton();

            // question & header
            char output[1024]{};
            auto questionEnd{ questionEndPos - msg };
            memcpy(output, msg, questionEnd);
            memcpy(output, &header, sizeof(header));

            char* outputPos{ output + questionEnd };
            // answer
            for (const std::shared_ptr<Record>& answer : answers)
                outputPos += memcpyRecord(outputPos, answer);

            // authority
            for (const auto& authority : authorities)
                outputPos += memcpyRecord(outputPos, authority);

            // additional
            for (const std::shared_ptr<Record>& additional : additionals)
                outputPos += memcpyRecord(outputPos, additional);
            outputPos += memcpyN(outputPos, remainingField, remainingFieldLength); // echo additional

            auto sendCount{ outputPos - &output[0] };
            print_hex(output, sendCount);
            sendto(m_socketFd, output, sendCount, 0, &pcliaddr, len);
        } else { // foreign DNS server
            int foreignServerSocket{ m_database.getForeignServerSocket() };
            send(foreignServerSocket, &msg, n, 0);

            char* receiveBuffer[1024]{};
            auto receiveCount{ recv(foreignServerSocket, &receiveBuffer, sizeof(receiveBuffer), 0) };
            sendto(m_socketFd, receiveBuffer, receiveCount, 0, &pcliaddr, len);
        }
    }
}

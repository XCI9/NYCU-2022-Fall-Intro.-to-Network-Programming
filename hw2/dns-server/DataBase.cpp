#include "DataBase.h"

using namespace DNSServer;

Record::Class DataBase::classStr2Int(std::string_view classString) {
    if (classString == "IN")
        return Record::Class::IN;
    else if (classString == "CS")
        return Record::Class::CS;
    else if (classString == "CH")
        return Record::Class::CH;
    else if (classString == "HS")
        return Record::Class::CS;

    fprintf(stderr, "should not reach here!\n");
    return Record::Class::IN;
}

Record::Type DataBase::typeStr2Int(std::string_view classString) {
    if (classString == "A")
        return Record::Type::A;
    else if (classString == "AAAA")
        return Record::Type::AAAA;
    else if (classString == "NS")
        return Record::Type::NS;
    else if (classString == "CNAME")
        return Record::Type::CNAME;
    else if (classString == "SOA")
        return Record::Type::SOA;
    else if (classString == "MX")
        return Record::Type::MX;
    else if (classString == "TXT")
        return Record::Type::TXT;

    fprintf(stderr, "should not reach here!\n");
    return Record::Type::A;
}

void DataBase::loadConfig(const char* path) {
    FILE* config{ fopen(path, "r") };

    char line[1024];
    fgets(line, sizeof(line), config);
    m_foreignServer = line;
    m_foreignServer.pop_back(); // remove \n

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, m_foreignServer.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(53);
    connect(m_foreignServerSocket, (sockaddr*)&servaddr, sizeof(servaddr));

    while (fgets(line, sizeof(line), config) != NULL) {
        char zoneName[1024];
        char zoneFilePath[1024];

        sscanf(line, "%[^,],%[^\n]\n", zoneName, zoneFilePath);

        char root[1024];
        sscanf(path, "%[^/]", root);

        std::string fullPath{ std::string{ root } + "/" + zoneFilePath };
        loadDomainInfo(fullPath.c_str());
    }
}

void DataBase::loadDomainInfo(const char* path) {
    FILE* zoneFile{ fopen(path, "r") };

    // domainName
    char line[1024];
    fgets(line, sizeof(line), zoneFile);

    std::string domainName{ line };
    domainName.pop_back();

    std::shared_ptr<Domain> domain{ std::make_shared<Domain>() };

    while (fgets(line, sizeof(line), zoneFile) != NULL) {
        std::vector<std::string_view> splitString{ split(line, ",") };

        std::string name{ splitString[0] };
        if (name == "@")
            name = "";

        uint32_t TTL;
        std::from_chars(splitString[1].data(), splitString[1].data() + splitString[1].size(), TTL); // string_view to int
        Record::Class classType = classStr2Int(splitString[2]);
        Record::Type type = typeStr2Int(splitString[3]);
        std::string data{ splitString[4] };

        if (data.back() == '\n')
            data.pop_back(); // remove '\n'
        if (data.back() == '.')
            data.pop_back(); // remove '.' at the end

        domain->addRecord(domainName, name, classType, type, TTL, data);
    }

    std::vector<std::string_view> domainNameSplit{ split(domainName, ".") };
    m_manager.insert(domainNameSplit, domain);
}

DataBase::DataBase(const char* path) : m_foreignServerSocket{ socket(AF_INET, SOCK_DGRAM, 0) } {
    loadConfig(path);
}

int DataBase::getForeignServerSocket() const { return m_foreignServerSocket; }

DomainManager& DataBase::operator()() { return m_manager; }

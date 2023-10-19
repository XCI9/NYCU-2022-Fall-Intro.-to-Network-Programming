#include <cstdio>
#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#pragma pack(1)

void swapByteOrder(uint32_t& ui)
{
    ui = (ui >> 24) |
        ((ui << 8) & 0x00FF0000) |
        ((ui >> 8) & 0x0000FF00) |
         (ui << 24);
}

void swapByteOrder(uint64_t& ull) {
    ull = (ull >> 56) |
         ((ull << 40) & 0x00FF000000000000) |
         ((ull << 24) & 0x0000FF0000000000) |
         ((ull << 8)  & 0x000000FF00000000) |
         ((ull >> 8)  & 0x00000000FF000000) |
         ((ull >> 24) & 0x0000000000FF0000) |
         ((ull >> 40) & 0x000000000000FF00) |
          (ull << 56);
}

struct Header {
    char magicBytes[4];
    uint32_t stringSection;
    uint32_t contentSection;
    uint32_t fileCount;
};

struct FileInfo {
    uint32_t nameOffset;
    uint32_t size;
    uint32_t contentOffset;
    uint64_t checksum;
};

int main(int argc, char* argv[]) {
    Header header{};
    
    std::ifstream originalFile{ "example.pak", std::ios::binary };
    originalFile.read(reinterpret_cast<char*>(&header), sizeof(header));

    printf("magic bytes: %.4s\n", &header.magicBytes);
    printf("String section offset: %d\n", header.stringSection);
    printf("Content section offset: %d\n", header.contentSection);
    printf("File count: %d\n\n", header.fileCount);

    //get file header
    std::vector<FileInfo> filesInfo;
    filesInfo.reserve(header.fileCount);
    for (std::size_t i{ 0 }; i < header.fileCount; i++) {
        FileInfo fileInfo{};

        //fread(&fileInfo, sizeof(fileInfo), 1, originalFile);
        originalFile.read(reinterpret_cast<char*>(&fileInfo), sizeof(fileInfo));

        //convert to little endian
        swapByteOrder(fileInfo.size);
        swapByteOrder(fileInfo.checksum);
        
        filesInfo.push_back(fileInfo);
    }

    printf("%11s %7s %13s %16s %11s %5s\n", "filenamePos", "size", "contentOffset", "checksum", "filename", "valid");
    for(const auto& file : filesInfo){
        printf("%11u %7u %13u %016llX", file.nameOffset, file.size, file.contentOffset, file.checksum);

        //switch to stringSection
        originalFile.seekg(header.stringSection, std::ios_base::beg);
        originalFile.seekg(file.nameOffset, std::ios_base::cur);

        std::string fileName;
        
        originalFile >> fileName;
        printf("%12s", fileName.c_str());

        //switch to contentSection
        originalFile.seekg(header.contentSection, std::ios_base::beg);
        originalFile.seekg(file.contentOffset, std::ios_base::cur);

        //calculate checksum of content
        uint64_t uncheckSum{ 0 };
        int currentFileSize{ static_cast<int>(file.size) };
        std::string contentBuffer;
        contentBuffer.reserve(file.size);
        while (currentFileSize > 0) {
            uint64_t checksumBlock{};
            originalFile.read(reinterpret_cast<char*>(&checksumBlock), std::min(static_cast<uint64_t>(currentFileSize),sizeof(checksumBlock)));
            contentBuffer.append(reinterpret_cast<char*>(&checksumBlock), sizeof(checksumBlock));
            uncheckSum ^= checksumBlock;
            currentFileSize -= 8;
        }

        //valid file
        if (uncheckSum == file.checksum) {
            std::ofstream unpackFile{ fileName, std::ios_base::binary};
            unpackFile << contentBuffer;
            printf("%6s\n", "yes");
        }
        else {
            printf("%6s\n", "no");
        }
    }
}

#include "StringOperation.h"

std::size_t memcpyN(void* destination, const void* source, std::size_t n) {
    memcpy(destination, source, n);
    return n;
}

std::size_t strncpyN(char* destination, const char* source, std::size_t n) {
    strncpy(destination, source, n);
    return n;
}

std::vector<std::string_view> split(std::string_view string, std::string_view splitter) {
    std::vector<std::string_view> splitString;
    for (const auto split : std::views::split(string, splitter))
        splitString.emplace_back(split.begin(), split.end());

    if (splitString.size() >= 1 && splitString.back() == "")
        splitString.pop_back();
    return splitString;
}

std::string combine(std::span<std::string_view> parts, char connector) {
    if (parts.size() == 0)
        return "";
    if (parts.size() == 1)
        return std::string{ parts[0] };

    std::string result;
    for (const auto& part : parts) {
        result += part;
        result += connector;
    }
    result.pop_back(); // remove last connector

    return result;
}
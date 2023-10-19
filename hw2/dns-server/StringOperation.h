#pragma once
#include <cstring>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// same as memcpy, but return the copy count
// i.e. Copy N bytes of SRC to DEST, return N
std::size_t memcpyN(void* destination, const void* source, std::size_t n);

// same as strncpy, but return the copy count
// i.e. Copy N bytes of SRC to DEST, return N
std::size_t strncpyN(char* destination, const char* source, std::size_t n);

// Input string, split this string using the Splitter, output a vector of split part
std::vector<std::string_view> split(std::string_view string, std::string_view splitter);

// Input some string Parts, concat them with the Connector
std::string combine(std::span<std::string_view> parts, char connector = '.');
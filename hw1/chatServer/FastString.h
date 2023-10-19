#pragma once
#include <cstring>
#include <string>
#include <string_view>

namespace FastString {
std::size_t size(const char* str);

std::size_t constexpr size(const char c);

std::size_t size(std::string_view str);

template <std::size_t N>
std::size_t constexpr size(char const (&)[N]);

template <typename... Strings>
std::size_t getTotalLength(Strings&&... str);

template <typename... Strings>
std::string concatenate(Strings&&... strings);
} // namespace FastString

// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
#include "FastString.tpp"
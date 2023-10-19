#include "FastString.h"

std::size_t FastString::size(const char* str) {
    return strlen(str);
}

std::size_t constexpr FastString::size(const char c) {
    return 1;
}

std::size_t FastString::size(std::string_view str) {
    return str.size();
}

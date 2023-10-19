template <std::size_t N>
std::size_t constexpr FastString::size(char const (&)[N]) {
    return N - 1;
}

template <typename... Strings>
std::size_t FastString::getTotalLength(Strings&&... str) {
    return (size(str) + ...);
}

template <typename... Strings>
std::string FastString::concatenate(Strings&&... strings) {
    std::size_t totalLength{ getTotalLength(strings...) };

    std::string output;
    output.reserve(totalLength);

    ((output += strings), ...);

    return output;
}
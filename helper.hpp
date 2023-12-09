#ifndef HELPER_HPP
#define HELPER_HPP

#include <charconv>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string_view>

constexpr auto splitString(const std::string_view data, const char delimiter) noexcept {
    return data | std::views::split(delimiter) | std::views::transform([](const auto& subRange) noexcept {
               return std::string_view{&*subRange.begin(), std::ranges::size(subRange)};
           });
}

void throwIfInvalid(bool valid, const char* msg = "Invalid Data");

inline std::optional<std::int64_t> convert(std::string_view input) {
    if ( !std::isdigit(input[0]) ) {
        return std::nullopt;
    } //if ( !std::isdigit(input[0]) )

    std::int64_t ret;
    auto         result = std::from_chars(input.begin(), input.end(), ret);
    throwIfInvalid(result.ec == std::errc{});
    return result.ptr == input.data() ? std::nullopt : std::optional{ret};
}

#endif //HELPER_HPP

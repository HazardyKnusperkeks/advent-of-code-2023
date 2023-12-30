#ifndef HELPER_HPP
#define HELPER_HPP

#include <charconv>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string_view>

template<bool SkipEmpty = true>
constexpr auto splitString(const std::string_view data, const char delimiter) noexcept {
    auto split = data | std::views::split(delimiter) | std::views::transform([](const auto& subRange) noexcept {
                     return std::string_view{&*subRange.begin(), std::ranges::size(subRange)};
                 });
    if constexpr ( SkipEmpty ) {
        return split | std::views::filter([](const std::string_view entry) noexcept { return !entry.empty(); });
    } //if constexpr ( SkipEmpty )
    else {
        return split;
    } //else -> if constexpr ( SkipEmpty )
}

void throwIfInvalid(bool valid, const char* msg = "Invalid Data");

template<int Base = 10>
inline std::optional<std::int64_t> convertOptionally(std::string_view input) {
    if ( Base == 10 && !std::isdigit(input[0]) && input[0] != '-' ) {
        return std::nullopt;
    } //if ( Base == 10 && !std::isdigit(input[0]) && input[0] != '-' )

    std::int64_t ret    = 0;
    auto         result = std::from_chars(input.begin(), input.end(), ret, Base);
    throwIfInvalid(result.ec == std::errc{});
    return result.ptr == input.data() ? std::nullopt : std::optional{ret};
}

template<int Base = 10>
inline std::int64_t convert(std::string_view input) {
    auto result = convertOptionally<Base>(input);
    throwIfInvalid(!!result);
    return *result;
}

inline double convertDouble(std::string_view input) {
    double ret    = 0.;
    auto   result = std::from_chars(input.begin(), input.end(), ret);
    throwIfInvalid(result.ec == std::errc{});
    throwIfInvalid(result.ptr != input.data());
    return ret;
}

#endif //HELPER_HPP

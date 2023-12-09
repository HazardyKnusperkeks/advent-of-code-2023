#ifndef HELPER_HPP
#define HELPER_HPP

#include <ranges>
#include <string_view>

constexpr auto splitString(const std::string_view data, const char delimiter) noexcept {
    return data | std::views::split(delimiter) | std::views::transform([](const auto& subRange) noexcept {
               return std::string_view{&*subRange.begin(), std::ranges::size(subRange)};
           });
}

#endif //HELPER_HPP

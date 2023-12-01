#include "challenge1.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>

using namespace std::string_view_literals;

namespace {
constexpr auto digits = "0123456789"sv;

int digifier(std::string_view input) {
    const auto firstDigitPos = input.find_first_of(digits);
    const auto lastDigitPos  = input.find_last_of(digits);

    if ( firstDigitPos == std::string_view::npos ) {
        throw std::runtime_error{std::format("Input \"{:s}\" does not contain a digit", input)};
    } //if ( if ( firstDigitPos == std::string_view::npos ) )

    const auto firstDigit = input[firstDigitPos] - '0';
    const auto lastDigit  = input[lastDigitPos] - '0';
    const auto result     = firstDigit * 10 + lastDigit;
    std::cout << std::setw(50) << std::left << input << " => " << result << '\n';
    return result;
}

int digifierWithStrings(std::string_view input) noexcept {
    constexpr std::array names{"one"sv, "two"sv,   "three"sv, "four"sv, "five"sv,
                               "six"sv, "seven"sv, "eight"sv, "nine"sv};
    constexpr auto       digitIndex = 9uz;
    std::array<int, 10>  positions;

    auto mapLow = [](std::string_view::size_type pos) noexcept {
        if ( pos == std::string_view::npos ) {
            return -1;
        } //if ( pos == std::string_view::npos )
        return static_cast<int>(pos);
    };

    auto mapHigh = [](std::string_view::size_type pos) noexcept {
        if ( pos == std::string_view::npos ) {
            return std::numeric_limits<int>::max();
        } //if ( pos == std::string_view::npos )
        return static_cast<int>(pos);
    };

    if ( input == "5jxjmnsn" ) {
        std::cout << ' ';
    }

    for ( auto&& [name, position] : std::views::zip(names, positions) ) {
        position = mapHigh(input.find(name));
    } //for ( auto& [name, positon] : std::views::zip(names, positions) )
    positions[digitIndex]      = mapHigh(input.find_first_of(digits));

    const auto firstDigitIter  = std::ranges::min_element(positions);
    const auto firstDigitIndex = firstDigitIter - positions.begin();
    const auto firstDigit = firstDigitIndex == digitIndex ? input[positions[digitIndex]] - '0' : firstDigitIndex + 1;

    for ( auto&& [name, position] : std::views::zip(names, positions) ) {
        auto posRange = std::ranges::find_end(input, name);
        position      = posRange.begin() == input.end() ? -1 : static_cast<int>(posRange.begin() - input.begin());
    } //for ( auto& [name, positon] : std::views::zip(names, positions) )
    positions[digitIndex]     = mapLow(input.find_last_of(digits));

    const auto lastDigitIter  = std::ranges::max_element(positions);
    const auto lastDigitIndex = lastDigitIter - positions.begin();
    const auto lastDigit      = lastDigitIndex == digitIndex ? input[positions[digitIndex]] - '0' : lastDigitIndex + 1;

    const auto result         = static_cast<int>(firstDigit * 10 + lastDigit);
    std::cout << std::setw(50) << std::left << input << " ==> " << result << '\n';
    return result;
}
} //namespace

void challenge1(const std::filesystem::path& dataDirectory) {
    const auto inputFilePath = dataDirectory / "1.txt";

    if ( !std::filesystem::exists(inputFilePath) ) {
        throw std::runtime_error{std::format("\"{:s}\" does not exist!", inputFilePath.c_str())};
    } //if ( !std::filesystem::exists(inputFilePath) )

    if ( !std::filesystem::is_regular_file(inputFilePath) ) {
        throw std::runtime_error{std::format("\"{:s}\" is not a file!", inputFilePath.c_str())};
    } //if ( !std::filesystem::is_regular_file(inputFilePath) )

    std::ifstream inputFile{inputFilePath};

    if ( !inputFile ) {
        throw std::runtime_error{std::format("Could not open \"{:s}\"!", inputFilePath.c_str())};
    } //if ( !inputFile )

    std::cout << " == Starting Challenge 1 ==\n";
    std::vector<std::string> input;
    std::ranges::copy(std::views::istream<std::string>(inputFile), std::back_inserter(input));

    auto       digified1 = input | std::views::transform(digifier);
    const auto sum1      = std::ranges::fold_left(digified1, 0, std::plus<>{});
    std::cout << "\n == Result of Challenge 1 Part: " << sum1 << " ==\n\n";

    auto       digified2 = input | std::views::transform(digifierWithStrings);
    const auto sum2      = std::ranges::fold_left(digified2, 0, std::plus<>{});
    std::cout << "\n == Result of Challenge 1 Part: " << sum1 << " ==\n == Result of Challenge 1 Part 2: " << sum2
              << " ==\n";

    return;
}

#include "challenge15.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <array>
#include <ranges>

using namespace std::string_view_literals;

namespace {
std::int64_t hash(std::string_view text) noexcept {
    std::int64_t ret = 0;
    for ( char c : text ) {
        ret += c;
        ret *= 17;
        ret %= 256;
    } //for ( char c : text )
    return ret;
}

using BoxContent = std::pair<std::string_view, int>;
using Box        = std::vector<BoxContent>;

std::int64_t calcLensPower(std::tuple<const BoxContent&, int> lensAndNumber) noexcept {
    return std::get<0>(lensAndNumber).second * std::get<1>(lensAndNumber);
}

std::int64_t calcBoxPower(std::tuple<const Box&, int> boxAndNumber) noexcept {
    return std::get<1>(boxAndNumber) *
           std::ranges::fold_left(std::views::zip(std::get<0>(boxAndNumber), std::views::iota(1)) |
                                      std::views::transform(calcLensPower),
                                  0, std::plus<>{});
}
} //namespace

bool challenge15(const std::vector<std::string_view>& input) {
    throwIfInvalid(input.size() == 1);
    std::int64_t sum1 =
        std::ranges::fold_left(splitString(input.front(), ',') | std::views::transform(hash), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::array<Box, 256> boxes;

    for ( std::string_view line : splitString(input.front(), ',') ) {
        const auto actionPos = line.find_first_of("=-"sv);
        throwIfInvalid(actionPos != std::string_view::npos);
        const auto label  = line.substr(0, actionPos);
        const char action = line[actionPos];
        const auto index  = hash(label);
        auto&      box    = boxes[static_cast<std::size_t>(index)];
        auto       iter   = std::ranges::find(box, label, &BoxContent::first);

        if ( action == '-' ) {
            if ( iter != box.end() ) {
                box.erase(iter);
            } //if ( iter != box.end() )
        } //if ( action == '-' )
        else {
            auto value = line.back() - '0';
            if ( iter == box.end() ) {
                box.push_back({label, value});
            } //if ( iter == box.end() )
            else {
                iter->second = value;
            } //else -> if ( iter == box.end() )
        } //else -> if ( action == '-' )
    } //for ( std::string_view line : splitString(input.front(), ',') )

    std::int64_t sum2 = std::ranges::fold_left(
        std::views::zip(boxes, std::views::iota(1)) | std::views::transform(calcBoxPower), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 506437 && sum2 == 288521;
}

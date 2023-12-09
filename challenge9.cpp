#include "challenge9.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>

namespace {
std::int64_t convert(std::string_view input) {
    std::int64_t ret;
    throwIfInvalid(std::from_chars(input.begin(), input.end(), ret).ec == std::errc{});
    return ret;
}

using Sequence = std::vector<std::int64_t>;

std::vector<Sequence> parse(const std::vector<std::string_view>& input) {
    std::vector<Sequence> sequences;

    for ( const auto& line : input ) {
        if ( line.empty() ) {
            continue;
        } //if ( line.empty() )

        auto& currentSequence = sequences.emplace_back();
        std::ranges::transform(splitString(line, ' '), std::back_inserter(currentSequence), convert);
    } //for ( const auto& line : input )
    return sequences;
}

template<typename Accessor, typename Combination>
std::int64_t extraPolateRow(const std::vector<std::int64_t>& row, const Accessor& accessor,
                            const Combination& combination) noexcept {
    std::vector<std::int64_t> nextRow(row.size() - 1);
    const auto                op = [](std::int64_t left, std::int64_t right) noexcept { return right - left; };
    std::ranges::transform(row, row | std::views::drop(1), nextRow.begin(), op);
    if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) ) {
        return std::invoke(accessor, row);
    } //if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) )
    const auto extraPolated = extraPolateRow(nextRow, accessor, combination);
    return combination(std::invoke(accessor, row), extraPolated);
}

std::int64_t extraPolateSequence(const Sequence& sequence) noexcept {
    return extraPolateRow(sequence, static_cast<const std::int64_t& (Sequence::*)() const>(&Sequence::back),
                          std::plus<>{});
}

std::int64_t extraPolateSequenceBackwards(const Sequence& sequence) noexcept {
    return extraPolateRow(sequence, static_cast<const std::int64_t& (Sequence::*)() const>(&Sequence::front),
                          std::minus<>{});
}
} //namespace

bool challenge9(const std::vector<std::string_view>& input) {
    const auto sequences = parse(input);

    const auto sum1 = std::ranges::fold_left(sequences | std::views::transform(extraPolateSequence), 0, std::plus<>{});

    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 =
        std::ranges::fold_left(sequences | std::views::transform(extraPolateSequenceBackwards), 0, std::plus<>{});

    myPrint(" == Result of Part 2: {:d} ==\n", sum2);
    return sum1 == 1'974'232'246 && sum2 == 928;
}

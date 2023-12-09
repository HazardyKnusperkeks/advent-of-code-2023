#include "challenge9.hpp"

#include "helper.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string_view>

using namespace std::string_view_literals;

namespace {
void throwIfInvalid(bool valid) {
    if ( !valid ) {
        throw std::runtime_error{"Invalid Data"};
    } //if ( !valid )
    return;
}

std::int64_t convert(std::string_view input) {
    std::int64_t ret;
    throwIfInvalid(std::from_chars(input.begin(), input.end(), ret).ec == std::errc{});
    return ret;
}

struct Sequence {
    std::vector<std::int64_t> Numbers;
};

std::vector<Sequence> parse(const std::vector<std::string_view>& input) {
    std::vector<Sequence> sequences;

    for ( const auto& line : input ) {
        if ( line.empty() ) {
            continue;
        } //if ( line.empty() )

        auto& currentSequence = sequences.emplace_back();
        std::ranges::transform(splitString(line, ' '), std::back_inserter(currentSequence.Numbers), convert);
    } //for ( const auto& line : input )
    return sequences;
}

std::int64_t extraPolateRow(const std::vector<std::int64_t>& row) noexcept {
    std::vector<std::int64_t> nextRow(row.size() - 1);
    const auto                op = [](std::int64_t left, std::int64_t right) noexcept { return right - left; };
    std::ranges::transform(row, row | std::views::drop(1), nextRow.begin(), op);
    if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) ) {
        return row.back();
    } //if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) )
    auto extraPolated = extraPolateRow(nextRow);
    return row.back() + extraPolated;
}

std::int64_t extraPolateSequence(const Sequence& sequence) noexcept {
    return extraPolateRow(sequence.Numbers);
}

std::int64_t extraPolateRowBackwards(const std::vector<std::int64_t>& row) noexcept {
    std::vector<std::int64_t> nextRow(row.size() - 1);
    const auto                op = [](std::int64_t left, std::int64_t right) noexcept { return right - left; };
    std::ranges::transform(row, row | std::views::drop(1), nextRow.begin(), op);
    if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) ) {
        return row.front();
    } //if ( std::ranges::all_of(nextRow, [](auto x) noexcept { return x == 0; }) )
    auto extraPolated = extraPolateRowBackwards(nextRow);
    return row.front() - extraPolated;
}

std::int64_t extraPolateSequenceBackwards(const Sequence& sequence) noexcept {
    return extraPolateRowBackwards(sequence.Numbers);
}
} //namespace

void challenge9(const std::vector<std::string_view>& input) {
    std::cout << " == Starting Challenge 9 ==\n";

    const auto sequences = parse(input);

    const auto sum1 = std::ranges::fold_left(sequences | std::views::transform(extraPolateSequence), 0, std::plus<>{});

    std::cout << " == Result of Challenge 9 Part 1: " << sum1 << " ==\n";

    const auto sum2 =
        std::ranges::fold_left(sequences | std::views::transform(extraPolateSequenceBackwards), 0, std::plus<>{});

    std::cout << " == Result of Challenge 9 Part 2: " << sum2 << " ==\n";
    return;
}

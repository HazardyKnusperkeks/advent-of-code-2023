#include "challenge13.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <utility>

namespace {
struct Pattern {
    std::vector<std::string> Rows;
    std::vector<std::string> Columns;
    std::int64_t             RowMirrored    = 0;
    std::int64_t             ColumnMirrored = 0;
};

auto parse(const std::vector<std::string_view>& input) {
    std::vector<Pattern> ret;
    Pattern*             currentPattern = &ret.emplace_back();
    for ( auto inputLine : input ) {
        if ( inputLine.empty() ) {
            currentPattern = &ret.emplace_back();
            continue;
        } //if ( inputLine.empty() )

        currentPattern->Rows.emplace_back(inputLine);
    } //for ( auto inputLine : input )
    return ret;
}

std::pair<std::string::iterator, std::string::iterator> theOneDifference(std::string& left,
                                                                         std::string& right) noexcept {
    auto inElementMismatch = std::ranges::mismatch(left, right);
    auto nextMismatch      = std::ranges::mismatch(std::ranges::subrange(std::next(inElementMismatch.in1), left.end()),
                                                   std::ranges::subrange(std::next(inElementMismatch.in2), right.end()));
    if ( nextMismatch.in1 == left.end() ) {
        return {inElementMismatch.in1, inElementMismatch.in2};
    } //if ( nextMismatch.in1 == left.end() )

    return {left.end(), right.end()};
}

template<bool RepairSmudge>
std::int64_t findMirrorInList(std::vector<std::string>& list) noexcept {
    const auto begin = list.begin();
    const auto end   = list.end();
    auto       iter  = begin;

    auto predicate   = [](std::string& left, std::string& right) noexcept {
        auto result = left == right;
        if ( result ) {
            return true;
        } //if ( result )

        if constexpr ( RepairSmudge ) {
            auto theOneMismatch = theOneDifference(left, right);
            if ( theOneMismatch.first != left.end() ) {
                return true;
            } //if ( theOneMismatch.first != left.end() )
        } //if constexpr ( RepairSmudge )

        return false;
    };

    for ( ;; ) {
        iter = std::ranges::adjacent_find(iter, end, predicate);

        if ( iter == end ) {
            return 0;
        } //if ( iter == end )

        auto mismatch = std::ranges::mismatch(std::ranges::subrange(begin, std::next(iter)) | std::views::reverse,
                                              std::ranges::subrange(std::next(iter), end));

        if ( mismatch.in1.base() == begin || mismatch.in2 == end ) {
            if constexpr ( !RepairSmudge ) {
                return std::distance(begin, iter) + 1;
            } //if constexpr ( !RepairSmudge )
            else {
                //Nichts zu reparieren, also weiter suchen.
                ++iter;
                continue;
            } //else -> if constexpr ( !RepairSmudge )
        } //if ( mismatch.in1.base() == begin || mismatch.in2 == end )

        if constexpr ( RepairSmudge ) {
            auto& leftRange      = *std::prev(mismatch.in1.base());
            auto& rightRange     = *mismatch.in2;
            auto  theOneMismatch = theOneDifference(leftRange, rightRange);

            if ( theOneMismatch.first != leftRange.end() ) {
                //Nur ein Unterschied!
                auto backup = std::exchange(*theOneMismatch.first, *theOneMismatch.second);

                 mismatch = std::ranges::mismatch(std::ranges::subrange(begin, std::next(iter)) | std::views::reverse,
                                                      std::ranges::subrange(std::next(iter), end));

                if ( mismatch.in1.base() == begin || mismatch.in2 == end ) {
                    return std::distance(begin, iter) + 1;
                } //if ( mismatch.in1.base() == begin || mismatch.in2 == end )

                //Nichts gefunden, ändere zurück und laufe weiter.
                *theOneMismatch.first = backup;
            } //if ( theOneMismatch.first != leftRange.end() )
        } //if constexpr ( RepairSmudge )

        ++iter;
    } //for ( ;; )
}

void transpose(Pattern& pattern) noexcept {
    auto rowCount    = pattern.Rows.size();
    auto columnCount = pattern.Rows.front().size();
    pattern.Columns.resize(columnCount);
    std::ranges::for_each(pattern.Columns, [rowCount](auto& column) noexcept {
        column.resize(rowCount);
        return;
    });

    for ( std::size_t columnIndex = 0; columnIndex != columnCount; ++columnIndex ) {
        for ( std::size_t rowIndex = 0; rowIndex != rowCount; ++rowIndex ) {
            pattern.Columns[columnIndex][rowIndex] = pattern.Rows[rowIndex][columnIndex];
        } //for ( std::size_t rowIndex = 0; rowIndex != rowCount; ++rowIndex )
    } //for ( std::size_t columnIndex = 0; columnIndex != columnCount; ++columnIndex )
    return;
}

template<bool RepairSmudge>
void findMirror(Pattern& pattern) noexcept {
    pattern.RowMirrored = findMirrorInList<RepairSmudge>(pattern.Rows);
    if ( pattern.RowMirrored != 0 ) {
        pattern.ColumnMirrored = 0;
        return;
    } //if ( pattern.RowMirrored != 0 )

    transpose(pattern);
    pattern.ColumnMirrored = findMirrorInList<RepairSmudge>(pattern.Columns);
    return;
}

std::int64_t mirrorPoints(const Pattern& pattern) noexcept {
    return pattern.ColumnMirrored + 100 * pattern.RowMirrored;
}
} //namespace

bool challenge13(const std::vector<std::string_view>& input) {
    auto patterns = parse(input);

    std::ranges::for_each(patterns, findMirror<false>);
    std::int64_t sum1 = std::ranges::fold_left(patterns | std::views::transform(mirrorPoints), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::ranges::for_each(patterns, findMirror<true>);
    std::int64_t sum2 = std::ranges::fold_left(patterns | std::views::transform(mirrorPoints), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 37025 && sum2 == 32854;
}

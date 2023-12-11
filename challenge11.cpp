#include "challenge11.hpp"

#include "print.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace {
using Row = std::string_view;
using Map = const std::vector<Row>&;

struct Coordinate {
    std::size_t Row;
    std::size_t Column;

    constexpr bool operator==(const Coordinate&) const noexcept = default;
};

std::vector<Coordinate> calcCoordinates(Map map) noexcept {
    std::vector<Coordinate> ret;
    for ( std::size_t rowIndex = 0; rowIndex < map.size(); ++rowIndex ) {
        auto row = map[rowIndex];
        for ( auto columnIndex = row.find('#'); columnIndex != std::string_view::npos;
              columnIndex      = row.find('#', columnIndex + 1) ) {
            ret.emplace_back(rowIndex, columnIndex);
        } //for ( auto columnIndex = row.find('#'); columnIndex != npos; columnIndex = row.find('#', columnIndex + 1) )
    } //for ( std::size_t rowIndex = 0; rowIndex < map.size(); ++rowIndex )
    return ret;
}

std::vector<std::size_t> calcEmptyRows(Map map) noexcept {
    std::vector<std::size_t> ret;
    std::size_t              emptyCounter = 0;
    for ( std::size_t rowIndex = 0; rowIndex < map.size(); ++rowIndex ) {
        auto row = map[rowIndex];
        if ( row.find('#') == std::string_view::npos ) {
            ++emptyCounter;
        } //if ( row.find('#') == std::string_view::npos )
        ret.emplace_back(emptyCounter);
    } //for ( std::size_t rowIndex = 0; rowIndex < map.size(); ++rowIndex )
    return ret;
}

std::vector<std::size_t> calcEmptyColumns(Map map) noexcept {
    std::vector<std::size_t> ret;
    std::size_t              emptyCounter = 0;
    for ( std::size_t columnIndex = 0; columnIndex < map[0].size(); ++columnIndex ) {
        if ( std::ranges::none_of(map, [columnIndex](Row row) noexcept { return row[columnIndex] == '#'; }) ) {
            ++emptyCounter;
        } //if ( std::ranges::none_of(map, [columnIndex](Row row) noexcept { return row[columnIndex] == '#'; }) )
        ret.emplace_back(emptyCounter);
    } //for ( std::size_t rowIndex = 0; rowIndex < map[0].size(); ++rowIndex )
    return ret;
}

std::int64_t calcDistance(const Coordinate& from, const Coordinate& to) noexcept {
    return std::abs(static_cast<std::int64_t>(from.Row) - static_cast<std::int64_t>(to.Row)) +
           std::abs(static_cast<std::int64_t>(from.Column) - static_cast<std::int64_t>(to.Column));
}
} //namespace

namespace std {
template<>
struct hash<Coordinate> {
    size_t operator()(const Coordinate& c) const noexcept {
        std::hash<std::size_t> h;
        return h(c.Row << 8) ^ h(c.Column);
    }
};
} //namespace std

bool challenge11(const std::vector<std::string_view>& input) {
    const auto& map               = input;
    const auto  galaxyCoordinates = calcCoordinates(map);
    const auto  emptyRows         = calcEmptyRows(map);
    const auto  emptyColumns      = calcEmptyColumns(map);

    std::vector<Coordinate> adaptedCoordinates(galaxyCoordinates);
    auto                    updateAdapted = [&adaptedCoordinates, &galaxyCoordinates, &emptyColumns,
                          &emptyRows](std::size_t factor) noexcept {
        std::ranges::transform(galaxyCoordinates, adaptedCoordinates.begin(),
                                                  [&emptyRows, &emptyColumns, factor](Coordinate c) noexcept {
                                   c.Row    += emptyRows[c.Row] * (factor - 1);
                                   c.Column += emptyColumns[c.Column] * (factor - 1);
                                   return c;
                               });
    };
    updateAdapted(2);

    auto calcSum = [&adaptedCoordinates, end = adaptedCoordinates.end()](void) noexcept {
        std::int64_t sum = 0;

        for ( auto iter = adaptedCoordinates.begin(); iter != end; ++iter ) {
            for ( auto nextIter = std::next(iter); nextIter != end; ++nextIter ) {
                sum += calcDistance(*iter, *nextIter);
            }
        }
        return sum;
    };
    const auto sum1 = calcSum();
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    updateAdapted(1000000);
    const auto sum2 = calcSum();

    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 9648398 && sum2 == 618'800'410'814;
}

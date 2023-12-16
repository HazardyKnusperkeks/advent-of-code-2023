#include "challenge16.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>

using namespace std::string_view_literals;

namespace {
enum class Direction { Up, Down, Left, Right };

struct Coordinate {
    std::size_t Row;
    std::size_t Column;

    constexpr bool operator==(const Coordinate&) const noexcept = default;

    Coordinate left(void) const noexcept {
        return {Row, Column - 1};
    }

    Coordinate right(void) const noexcept {
        return {Row, Column + 1};
    }

    Coordinate up(void) const noexcept {
        return {Row - 1, Column};
    }

    Coordinate down(void) const noexcept {
        return {Row + 1, Column};
    }
};

using CoordinateAndDirection = std::pair<Coordinate, Direction>;
} //namespace

namespace std {
template<>
struct hash<Coordinate> {
    size_t operator()(const Coordinate& c) const noexcept {
        std::hash<std::size_t> h;
        return h(c.Row << 8) ^ h(c.Column);
    }
};

template<>
struct hash<CoordinateAndDirection> {
    size_t operator()(const CoordinateAndDirection& cd) const noexcept {
        std::hash<Coordinate> c;
        std::hash<Direction>  d;
        return (c(cd.first) << 8) ^ d(cd.second);
    }
};
} //namespace std

namespace {
struct Energizer {
    const std::vector<std::string_view>& Map;
    const std::size_t                    RowCount;
    const std::size_t                    ColumnCount;

    std::unordered_set<Coordinate>                           AlreadyEnergized;
    std::unordered_set<CoordinateAndDirection>               AlreadyVisited;

    Energizer(const std::vector<std::string_view>& map) noexcept :
            Map{map}, RowCount{map.size()}, ColumnCount{map.front().size()} {
        return;
    }

    std::int64_t startEnergize(Coordinate pos = {0, 0}, Direction dir = Direction::Right) noexcept {
        AlreadyVisited.clear();
        AlreadyEnergized.clear();
        return energize(pos, dir);
    }

    std::int64_t energize(Coordinate pos, Direction dir) noexcept {
        if ( !AlreadyVisited.insert({pos, dir}).second ) {
            return 0;
        } //if ( !AlreadyVisited.insert({pos, dir}).second )

        std::int64_t res = AlreadyEnergized.insert(pos).second ? 1 : 0;
        switch ( dir ) {
            case Direction::Left  : res += moveLeft(pos); break;
            case Direction::Right : res += moveRight(pos); break;
            case Direction::Up    : res += moveUp(pos); break;
            case Direction::Down  : res += moveDown(pos); break;
        } //switch ( dir )
        // Cache.emplace(CoordinateAndDirection{pos, dir}, res);
        return res;
    }

    std::int64_t checkValidAndEnergize(Coordinate pos, Direction dir) noexcept {
        if ( pos.Row >= RowCount || pos.Column >= ColumnCount ) {
            return 0;
        } //if ( pos.Row >= RowCount || pos.Column >= ColumnCount )
        return energize(pos, dir);
    }

    char mapEntry(Coordinate pos) noexcept {
        return Map[pos.Row][pos.Column];
    }

    std::int64_t moveLeft(Coordinate pos) noexcept {
        switch ( mapEntry(pos) ) {
            case '|' : {
                return checkValidAndEnergize(pos.up(), Direction::Up) +
                       checkValidAndEnergize(pos.down(), Direction::Down);
            } //case '|'

            case '-' :
            case '.'  : return checkValidAndEnergize(pos.left(), Direction::Left);

            case '\\' : return checkValidAndEnergize(pos.up(), Direction::Up);
            case '/'  : return checkValidAndEnergize(pos.down(), Direction::Down);
        } //switch ( mapEntry(pos) )
        throwIfInvalid(false);
        return 0;
    }

    std::int64_t moveRight(Coordinate pos) noexcept {
        switch ( mapEntry(pos) ) {
            case '|' : {
                return checkValidAndEnergize(pos.up(), Direction::Up) +
                       checkValidAndEnergize(pos.down(), Direction::Down);
            } //case '|'

            case '-' :
            case '.'  : return checkValidAndEnergize(pos.right(), Direction::Right);

            case '/'  : return checkValidAndEnergize(pos.up(), Direction::Up);
            case '\\' : return checkValidAndEnergize(pos.down(), Direction::Down);
        } //switch ( mapEntry(pos) )
        throwIfInvalid(false);
        return 0;
    }

    std::int64_t moveUp(Coordinate pos) noexcept {
        switch ( mapEntry(pos) ) {
            case '-' : {
                return checkValidAndEnergize(pos.left(), Direction::Left) +
                       checkValidAndEnergize(pos.right(), Direction::Right);
            } //case '-'

            case '|' :
            case '.'  : return checkValidAndEnergize(pos.up(), Direction::Up);

            case '/'  : return checkValidAndEnergize(pos.right(), Direction::Right);
            case '\\' : return checkValidAndEnergize(pos.left(), Direction::Left);
        } //switch ( mapEntry(pos) )
        throwIfInvalid(false);
        return 0;
    }

    std::int64_t moveDown(Coordinate pos) noexcept {
        switch ( mapEntry(pos) ) {
            case '-' : {
                return checkValidAndEnergize(pos.left(), Direction::Left) +
                       checkValidAndEnergize(pos.right(), Direction::Right);
            } //case '-'

            case '|' :
            case '.'  : return checkValidAndEnergize(pos.down(), Direction::Down);

            case '\\' : return checkValidAndEnergize(pos.right(), Direction::Right);
            case '/'  : return checkValidAndEnergize(pos.left(), Direction::Left);
        } //switch ( mapEntry(pos) )
        throwIfInvalid(false);
        return 0;
    }
};
} //namespace

bool challenge16(const std::vector<std::string_view>& input) {
    throwIfInvalid(!input.empty());

    Energizer    energizer{input};
    std::int64_t sum1 = energizer.startEnergize();
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::int64_t sum2 = std::ranges::max(
        std::views::iota(0uz, energizer.ColumnCount) | std::views::transform([&energizer](std::size_t column) noexcept {
            return std::max(energizer.startEnergize({0, column}, Direction::Down),
                            energizer.startEnergize({energizer.RowCount - 1, column}, Direction::Up));
        }));
    sum2 = std::max(sum2, std::ranges::max(std::views::iota(0uz, energizer.RowCount) |
                                           std::views::transform([&energizer](std::size_t row) noexcept {
                                               return std::max(energizer.startEnergize({row, 0}, Direction::Right),
                                                               energizer.startEnergize({row, energizer.ColumnCount - 1},
                                                                                       Direction::Left));
                                           })));
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 8021 && sum2 == 8216;
}

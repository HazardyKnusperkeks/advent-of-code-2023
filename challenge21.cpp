#include "challenge21.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_set>

using namespace std::string_view_literals;

namespace {
template<typename T>
struct Coordinate {
    T Row;
    T Column;

    constexpr bool operator==(const Coordinate&) const noexcept  = default;
    constexpr auto operator<=>(const Coordinate&) const noexcept = default;

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
} //namespace

namespace std {
template<typename T>
struct hash<Coordinate<T>> {
    size_t operator()(const Coordinate<T>& c) const noexcept {
        constexpr auto bits = std::numeric_limits<T>::digits / 2;
        constexpr auto mask = ((T{1} << bits) - 1);
        return std::hash<T>{}((c.Row << bits) | (c.Column & mask));
    }
};

template<typename T, typename U>
struct hash<std::pair<Coordinate<T>, U>> {
    size_t operator()(const std::pair<Coordinate<T>, U>& p) const noexcept {
        return std::hash<Coordinate<T>>{}(p.first) ^ std::hash<U>{}(p.second);
    }
};
} //namespace std

namespace {
using MyCoordinate     = Coordinate<std::int64_t>;
using Map              = const std::vector<std::string_view>&;

std::int64_t MaxColumn = 0;
std::int64_t MaxRow    = 0;

bool isValid(const MyCoordinate& coordinate) noexcept {
    return coordinate.Column >= 0 && coordinate.Column < MaxColumn && coordinate.Row >= 0 && coordinate.Row < MaxRow;
}

MyCoordinate findStart(Map map) noexcept {
    MyCoordinate ret;
    for ( ret.Row = 0; ret.Row < MaxRow; ++ret.Row ) {
        ret.Column = static_cast<std::int64_t>(map[static_cast<std::size_t>(ret.Row)].find('S'));

        if ( static_cast<std::size_t>(ret.Column) != std::string::npos ) {
            return ret;
        } //if ( static_cast<std::size_t>(ret.Column) != std::string::npos )
    } //for ( ret.Row = 0; ret.Row < MaxRow; ++ret.Row )
    return {};
}

template<bool Infinite>
struct ReachableCalculator {
    ::Map Map;

    ReachableCalculator(::Map map) noexcept : Map{map} {
        return;
    }

    std::int64_t calcReachable(MyCoordinate start, std::int64_t totalSteps) noexcept {
        std::unordered_set<MyCoordinate> alreadySeen;
        std::array<std::int64_t, 2>      counter{};

        std::vector<MyCoordinate> current;
        std::vector<MyCoordinate> next{start};

        //Step 0 ist die Start Position.
        for ( std::size_t step = 0; step <= static_cast<std::size_t>(totalSteps); ++step ) {
            std::swap(current, next);
            next.clear();

            for ( auto coordinate : current ) {
                const auto normalizedCoordinate = [](MyCoordinate c) noexcept {
                    if constexpr ( !Infinite ) {
                        return c;
                    } //if constexpr ( !Infinite )
                    else {
                        while ( c.Row < 0 ) {
                            c.Row += MaxRow;
                        } //while ( c.Row < 0 )
                        c.Row %= MaxRow;

                        while ( c.Column < 0 ) {
                            c.Column += MaxColumn;
                        } //while ( c.Column < 0 )
                        c.Column %= MaxColumn;

                        return c;
                    } //else -> if constexpr( !Infinite )
                }(coordinate);

                if constexpr ( !Infinite ) {
                    if ( !isValid(coordinate) ) {
                        continue;
                    } //if ( !isValid(coordinate) )
                } //if constexpr ( !Infinite )

                const auto plot = Map[static_cast<std::size_t>(normalizedCoordinate.Row)]
                                     [static_cast<std::size_t>(normalizedCoordinate.Column)];

                if ( plot == '#' ) {
                    continue;
                } //if ( plot == '#' )

                if ( !alreadySeen.insert(coordinate).second ) {
                    continue;
                } //if ( !alreadySeen.insert(coordinate).second )
                alreadySeen.insert(coordinate);
                ++counter[step % 2];

                next.push_back(coordinate.down());
                next.push_back(coordinate.up());
                next.push_back(coordinate.left());
                next.push_back(coordinate.right());
            } //for ( auto coordinate : current )
        } //for ( int step = 0; step <= totalSteps; ++step )

        return counter[static_cast<std::size_t>(totalSteps) % 2];
    }
};
} //namespace

bool challenge21(Map map) {
    throwIfInvalid(!map.empty());

    MaxRow           = static_cast<std::int64_t>(map.size());
    MaxColumn        = static_cast<std::int64_t>(map.front().size());
    const auto start = findStart(map);
    throwIfInvalid(MaxColumn == MaxRow);
    throwIfInvalid(start.Row == MaxRow / 2);
    throwIfInvalid(start.Column == MaxColumn / 2);

    ReachableCalculator<false> partOneCalculator{map};
    auto                       reachable = partOneCalculator.calcReachable(start, 64);
    myPrint(" == Result of Part 1: {:d} ==\n", reachable);

    ReachableCalculator<true> partTwoCalculator{map};
    auto                      calcPart2 = [&partTwoCalculator, &start](int steps) noexcept {
        //Extrapolation über ein quadratisches Polynom. Nicht das ich selbst auf die Idee gekommen wäre...
        auto y1     = partTwoCalculator.calcReachable(start, start.Row);
        auto y2     = partTwoCalculator.calcReachable(start, MaxRow + start.Row);
        auto y3     = partTwoCalculator.calcReachable(start, MaxRow * 2 + start.Row);

        auto a      = (y3 + y1 - 2 * y2) / 2;
        auto b      = (4 * y2 - 3 * y1 - y3) / 2;
        auto c      = y1;
        auto evalAt = (steps - start.Row) / MaxRow;
        auto ret    = (a * evalAt * evalAt) + (b * evalAt) + c;
        return ret;
    };
    auto reachable2 = calcPart2(26501365);
    myPrint(" == Result of Part 2: {:d} ==\n", reachable2);

    return reachable == 3858 && reachable2 == 636'350'496'972'143;
}

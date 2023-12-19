#include "challenge18.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <ranges>

using namespace std::string_view_literals;

namespace {
enum class Direction : std::int8_t { Up, Left, Down, Right };

Direction turnLeft(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 4 - 1) % 4);
}

Direction turnRight(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 1) % 4);
}

Direction turnAround(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 2) % 4);
}

bool isUpDown(Direction d) noexcept {
    using enum Direction;
    return d == Up || d == Down;
}

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

    Coordinate& move(Direction direction, T distance = 1) noexcept {
        switch ( direction ) {
            using enum Direction;
            case Left  : Column -= distance; break;
            case Up    : Row -= distance; break;
            case Right : Column += distance; break;
            case Down  : Row += distance; break;
        } //switch ( direction )
        return *this;
    }

    Coordinate moved(Direction direction, T distance = 1) const noexcept {
        auto ret = *this;
        ret.move(direction, distance);
        return ret;
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
} //namespace std

namespace {
struct DigAction {
    ::Direction  Direction;
    std::int64_t Length;
};

using ParseResult = std::vector<DigAction>;

template<bool UseColor>
ParseResult parse(const std::vector<std::string_view>& input) {
    ParseResult ret;
    ret.reserve(input.size());
    for ( const auto line : input ) {
        auto match = ctre::match<R"(([ULRD]) (\d+) \(#([0-9a-fA-F]{5})([0-3])\))">(line);
        throwIfInvalid(match);

        auto [direction, length] = [&match](void) {
            if constexpr ( UseColor ) {
                return std::pair{[](const char c) noexcept {
                                     return static_cast<Direction>(3 - (c - '0'));
                                 }(static_cast<std::string_view>(match.get<4>()).front()),
                                 static_cast<std::size_t>(convert<16>(match.get<3>()))};
            } //if constexpr ( UseColor )
            else {
                return std::pair{[](const char c) noexcept {
                                     switch ( c ) {
                                         using enum Direction;
                                         case 'U' : return Up;
                                         case 'D' : return Down;
                                         case 'L' : return Left;
                                         default  : return Right;
                                     } //switch ( c )
                                 }(static_cast<std::string_view>(match.get<1>()).front()),
                                 static_cast<std::size_t>(convert(match.get<2>()))};
            } //else -> if constexpr( UseColor )
        }();

        ret.emplace_back(direction, length);
    } //for ( const auto line : input )
    return ret;
}

std::int64_t dig(const ParseResult& input) noexcept {
    const auto coordinateAndMove = [current = Coordinate<std::int64_t>{0, 0}](const DigAction& dig) mutable noexcept {
        return current.move(dig.Direction, dig.Length);
    };

    const auto calcDeterminante = [](const Coordinate<std::int64_t>& left,
                                     const Coordinate<std::int64_t>& right) noexcept {
        return (left.Row + right.Row) * (left.Column - right.Column);
    };

    std::vector<Coordinate<std::int64_t>> coordinates;
    coordinates.resize(input.size());
    std::ranges::transform(input, coordinates.begin(), coordinateAndMove);
    const auto innerArea = std::ranges::fold_left(std::views::zip_transform(calcDeterminante, coordinates,
                                                                            coordinates | std::views::drop(1)),
                                                  0, std::plus<>{}) /
                           2;
    const auto perimeter = std::ranges::fold_left(input | std::views::transform(&DigAction::Length), 0, std::plus<>{});
    return innerArea + perimeter / 2 + 1;
}
} //namespace

bool challenge18(const std::vector<std::string_view>& rawInput) {
    auto area1 = dig(parse<false>(rawInput));
    myPrint(" == Result of Part 1: {:d} ==\n", area1);

    auto area2 = dig(parse<true>(rawInput));
    myPrint(" == Result of Part 2: {:d} ==\n", area2);

    return area1 == 40714 && area2 == 129'849'166'997'110;
}

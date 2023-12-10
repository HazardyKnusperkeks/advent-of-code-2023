#include "challenge10.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <unordered_map>
#include <utility>

namespace {
enum PipeDirection : char {
    NorthSouth = '|',
    WestEast   = '-',
    NorthEast  = 'L',
    NorthWest  = 'J',
    SouthWest  = '7',
    SouthEast  = 'F',
    Ground     = '.',
    Animal     = 'S'
};

using Row = std::string_view;
using Map = const std::vector<Row>&;

struct Coordinate {
    std::size_t Row;
    std::size_t Column;

    constexpr bool operator==(const Coordinate&) const noexcept = default;
};

struct MovingPosition {
    Coordinate Current;
    Coordinate Previous;
};

Coordinate findAnimal(Map map) {
    Coordinate ret;
    for ( ret.Row = 0; ret.Row < map.size(); ++ret.Row ) {
        ret.Column = map[ret.Row].find(PipeDirection::Animal);
        if ( ret.Column != std::string_view::npos ) {
            return ret;
        } //if ( ret.Column != std::string_view::npos )
    } //for ( ret.Row = 0; ret.Row < map.size(); ++ret.Row )
    throwIfInvalid(false);
    return {};
}

bool isNorth(PipeDirection d) noexcept {
    return d == NorthSouth || d == NorthEast || d == NorthWest;
}

bool isSouth(PipeDirection d) noexcept {
    return d == NorthSouth || d == SouthEast || d == SouthWest;
}

bool isWest(PipeDirection d) noexcept {
    return d == WestEast || d == NorthWest || d == SouthWest;
}

bool isEast(PipeDirection d) noexcept {
    return d == WestEast || d == NorthEast || d == SouthEast;
}

PipeDirection directionFrom(int direction) {
    switch ( direction ) {
        case 0x03 : return NorthWest;
        case 0x05 : return NorthEast;
        case 0x06 : return WestEast;
        case 0x09 : return NorthSouth;
        case 0x0A : return SouthWest;
        case 0x0C : return SouthEast;
    } //switch ( direction )
    throwIfInvalid(false);
    return Ground;
}

auto findNeighbors(Map map, Coordinate start) {
    std::array<Coordinate, 2> ret;
    std::size_t               found      = 0;
    const bool                checkNorth = start.Row > 0;
    const bool                checkSouth = start.Row < map.size() - 1;
    const bool                checkWest  = start.Column > 0;
    const bool                checkEast  = start.Column < map[0].size() - 1;
    int                       direction  = 0;

    for ( int neighbor = 0; neighbor < 4 && found != 2; ++neighbor ) {
        switch ( neighbor ) {
            case 0 : {
                if ( checkNorth && isSouth(static_cast<PipeDirection>(map[start.Row - 1][start.Column])) ) {
                    ret[found] = {start.Row - 1, start.Column};
                    ++found;
                    direction |= 0x01;
                } //if ( checkNorth && isSouth(static_cast<PipeDirection>(map[start.Row - 1][start.Column]))
                break;
            } //case 0

            case 1 : {
                if ( checkWest && isEast(static_cast<PipeDirection>(map[start.Row][start.Column - 1])) ) {
                    ret[found] = {start.Row, start.Column - 1};
                    ++found;
                    direction |= 0x02;
                } //if ( checkWest && isEast(static_cast<PipeDirection>(map[start.Row][start.Column - 1])) )
                break;
            } //case 1

            case 2 : {
                if ( checkEast && isWest(static_cast<PipeDirection>(map[start.Row][start.Column + 1])) ) {
                    ret[found] = {start.Row, start.Column + 1};
                    ++found;
                    direction |= 0x04;
                } //if ( checkEast && isWest(static_cast<PipeDirection>(map[start.Row][start.Column + 1])) )
                break;
            } //case 2

            case 3 : {
                if ( checkSouth && isNorth(static_cast<PipeDirection>(map[start.Row + 1][start.Column])) ) {
                    ret[found] = {start.Row + 1, start.Column};
                    ++found;
                    direction |= 0x08;
                } //if ( checkSouth && isNorth(static_cast<PipeDirection>(map[start.Row + 1][start.Column])) )
                break;
            } //case 3
        } //switch ( neighbor )
    } //for ( int neighbor = 0; neighbor < 9 && found != 2; ++neighbor )
    throwIfInvalid(found == 2);
    return std::pair{ret, directionFrom(direction)};
}

void move(Map map, MovingPosition& pos) {
    auto previous = std::exchange(pos.Previous, pos.Current);

    switch ( static_cast<PipeDirection>(map[pos.Current.Row][pos.Current.Column]) ) {
        case Animal :
        case Ground     : throwIfInvalid(false); break;

        case NorthSouth : pos.Current.Row += pos.Current.Row - previous.Row; break;
        case WestEast   : pos.Current.Column += pos.Current.Column - previous.Column; break;

        case NorthWest  : {
            if ( pos.Current.Row == previous.Row ) {
                --pos.Current.Row;
            } //if ( pos.Current.Row == previous.Row )
            else {
                --pos.Current.Column;
            } //else -> if ( pos.Current.Row == previous.Row )
            break;
        } //case NorthWest

        case NorthEast : {
            if ( pos.Current.Row == previous.Row ) {
                --pos.Current.Row;
            } //if ( pos.Current.Row == previous.Row )
            else {
                ++pos.Current.Column;
            } //else -> if ( pos.Current.Row == previous.Row )
            break;
        } //case NorthEast

        case SouthWest : {
            if ( pos.Current.Row == previous.Row ) {
                ++pos.Current.Row;
            } //if ( pos.Current.Row == previous.Row )
            else {
                --pos.Current.Column;
            } //else -> if ( pos.Current.Row == previous.Row )
            break;
        } //case SouthWest

        case SouthEast : {
            if ( pos.Current.Row == previous.Row ) {
                ++pos.Current.Row;
            } //if ( pos.Current.Row == previous.Row )
            else {
                ++pos.Current.Column;
            } //else -> if ( pos.Current.Row == previous.Row )
            break;
        } //case SouthEast
    } //switch ( static_cast<PipeDirection>(map[pos.Current.Row][pos.Current.Column]) )
    return;
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

bool challenge10(const std::vector<std::string_view>& input) {
    const auto& map                              = input;
    const auto  animalPosition                   = findAnimal(map);
    const auto [startPositions, animalDirection] = findNeighbors(map, animalPosition);

    MovingPosition pos1{startPositions[0], animalPosition};
    MovingPosition pos2{startPositions[1], animalPosition};

    std::unordered_map<Coordinate, std::int64_t> partOfLoop;
    partOfLoop.emplace(animalPosition, 0);
    partOfLoop.emplace(startPositions[0], 1);
    partOfLoop.emplace(startPositions[1], 1);

    std::int64_t moves = 1;
    while ( pos1.Current != pos2.Current ) {
        move(map, pos1);
        move(map, pos2);
        ++moves;
        partOfLoop.emplace(pos1.Current, moves);
        partOfLoop.emplace(pos2.Current, moves);
    } //while ( pos1.Current != pos2.Current )

    myPrint(" == Result of Part 1: {:d} ==\n", moves);

    std::int64_t withinLoop = 0;
    const auto   end        = partOfLoop.end();
    for ( std::size_t row = 0; row < map.size(); ++row ) {
        bool within    = false;
        bool fromNorth = false;

        for ( Coordinate pos{row, 0}; pos.Column < map[row].size(); ++pos.Column ) {
            if ( auto iter = partOfLoop.find(pos); iter != end ) {
                auto direction = static_cast<PipeDirection>(map[pos.Row][pos.Column]);
                if ( direction == Animal ) {
                    direction = animalDirection;
                } //if ( direction == Animal )

                if ( within ) {
                    switch ( direction ) {
                        case Animal     :
                        case Ground     : throwIfInvalid(false); break;
                        case WestEast   : break;
                        case NorthSouth : within = false; break;
                        case NorthEast  : fromNorth = true; break;
                        case SouthWest  : within = !fromNorth; break;
                        case SouthEast  : fromNorth = false; break;
                        case NorthWest  : within = fromNorth; break;
                    } //switch ( direction )
                } //if ( within )
                else {
                    switch ( direction ) {
                        case Animal     :
                        case Ground     : throwIfInvalid(false); break;
                        case WestEast   : break;
                        case NorthSouth : within = true; break;
                        case NorthEast  : fromNorth = true; break;
                        case SouthWest  : within = fromNorth; break;
                        case SouthEast  : fromNorth = false; break;
                        case NorthWest  : within = !fromNorth; break;
                    } //switch ( direction )
                } //else -> if ( within )
            } //if ( partOfLoop.cotains(pos) )
            else {
                if ( within ) {
                    ++withinLoop;
                } //if ( within )
            } //else -> if ( partOfLoop.contains(pos) )
        } //for ( std::size_t column = 0; pos.Column < map[row].size(); ++pos.Column )

        throwIfInvalid(!within);
    } //for ( std::size_t row = 0; row < map.size(); ++row )

    myPrint(" == Result of Part 2: {:d} ==\n", withinLoop);

    return moves == 6786 && withinLoop == 495;
}

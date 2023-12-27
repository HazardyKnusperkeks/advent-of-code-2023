#include "challenge23.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <memory>
#include <ranges>
#include <unordered_map>

namespace {
enum class Direction : std::int8_t { Up, Left, Down, Right };

Direction turnRight(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 4 - 1) % 4);
}

Direction turnLeft(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 1) % 4);
}

Direction turnAround(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 2) % 4);
}

bool isUpDown(Direction d) noexcept {
    using enum Direction;
    return d == Up || d == Down;
}

auto directionRange(void) noexcept {
    return std::views::iota(0, 4) | std::views::transform([](int i) noexcept { return static_cast<Direction>(i); });
}

auto directionRangeWithout(Direction blocked) noexcept {
    return std::views::iota(0, 4) | std::views::transform([](int i) noexcept { return static_cast<Direction>(i); }) |
           std::views::filter([blocked](Direction d) noexcept { return d != blocked; });
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
struct Node {
    std::array<Node*, 2> Successors{nullptr, nullptr};
    std::array<Node*, 2> Predecessors{nullptr, nullptr};
    std::int32_t         LongestPathFromTarget{0};

    void addSuccessor(Node* successor) {
        if ( !Successors[0] ) {
            Successors[0] = successor;
            return;
        } //if ( !Successors[0] )

        throwIfInvalid(Successors[0] != successor);
        throwIfInvalid(!Successors[1]);
        Successors[1] = successor;
        return;
    }
};

struct Crossing;

struct Pathway {
    std::int32_t Length = 1;
    Crossing*    Begin  = nullptr;
    Crossing*    End    = nullptr;
};

struct Crossing {
    std::array<Pathway*, 4> Pathways{};
    std::int32_t            LongestPathFromTarget = 0;
};

struct Graph {
    using Coordinate = ::Coordinate<std::size_t>;

    Pathway*                                                  StartPathway;
    Pathway*                                                  TargetPathway;
    std::vector<std::unique_ptr<Pathway>>                     Pathways;
    std::unordered_map<Coordinate, Pathway*>                  PathwayMap;
    std::unordered_map<Coordinate, std::unique_ptr<Crossing>> Crossings;
    std::size_t                                               MaxRow;
    std::size_t                                               MaxColumn;

    Graph(const std::vector<std::string_view>& input) :
            StartPathway{nullptr}, TargetPathway{nullptr}, MaxRow{input.size()},
            MaxColumn{(throwIfInvalid(MaxRow > 2), input.front().size())} {
        const auto firstLine = input.front();
        const auto lastLine  = input.back();

        const Coordinate startCoordinate{0, firstLine.find('.')};
        const Coordinate targetCoordinate{MaxRow - 1, lastLine.find('.')};
        throwIfInvalid(startCoordinate.Column != std::string_view::npos);
        throwIfInvalid(targetCoordinate.Column != std::string_view::npos);
        StartPathway =
            buildPathway<true>(input, startCoordinate, Direction::Up, /*fromCrossing=*/nullptr, targetCoordinate);
        throwIfInvalid(TargetPathway);
        //Das erste Feld ist kein Schritt.
        --StartPathway->Length;
        return;
    }

    template<bool PartOne>
    std::int32_t calculateLongestPath(void) noexcept {
        std::vector<Crossing*> visited;
        return calculateLongestPath<PartOne>(TargetPathway->Begin, TargetPathway, visited);
    }

    template<bool PartOne>
    std::int32_t calculateLongestPath(Crossing* currentCrossing, Pathway* incomingPathway,
                                      std::vector<Crossing*>& visited) noexcept {
        auto iter = std::ranges::lower_bound(visited, currentCrossing);
        visited.insert(iter, currentCrossing);
        const auto blockedDirection = directionOfPathway(incomingPathway, currentCrossing);
        const auto isUpOrLeft       = [](Direction direction) noexcept {
            return direction == Direction::Up || direction == Direction::Left;
        };
        const auto filter = [&isUpOrLeft](Direction direction) noexcept {
            if constexpr ( PartOne ) {
                return isUpOrLeft(direction);
            } //if constexpr ( PartOne )
            static_cast<void>(isUpOrLeft);
            return true;
        };

        std::int32_t max = 0;

        for ( auto direction : directionRangeWithout(blockedDirection) | std::views::filter(filter) ) {
            auto pathway = currentCrossing->Pathways[static_cast<std::size_t>(direction)];

            if ( !pathway ) {
                continue;
            } //if ( !pathway )

            Crossing* nextCrossing = isUpOrLeft(direction) ? pathway->Begin : pathway->End;
            if ( !nextCrossing ) {
                max = pathway->Length;
                break;
            } //if ( !nextCrossing )

            if ( std::ranges::binary_search(visited, nextCrossing) ) {
                continue;
            } //if ( std::ranges::binary_search(visited, nextCrossing) )

            max = std::max(max, calculateLongestPath<PartOne>(nextCrossing, pathway, visited));
        } //for ( auto direction : directionRangeWithout(blockedDirection) | std::views::filter(filter) )
        iter = std::ranges::lower_bound(visited, currentCrossing);
        throwIfInvalid(*iter == currentCrossing);
        visited.erase(iter);
        return max + 1 + incomingPathway->Length;
    }

    private:
    bool isValid(Coordinate c) const noexcept {
        return c.Row < MaxRow && c.Column < MaxColumn;
    }

    template<bool AssignStart>
    Pathway* buildPathway(const std::vector<std::string_view>& input, Coordinate pathwayStart,
                          Direction blockedDirection, Crossing* fromCrossing, Coordinate targetCoordinate) {
        auto field = input[pathwayStart.Row][pathwayStart.Column];

        if ( field == '#' ) {
            return nullptr;
        } //if ( field == '#' )

        if ( auto iter = PathwayMap.find(pathwayStart); iter != PathwayMap.end() ) {
            return iter->second;
        } //if ( auto iter = PathwayMap.find(pathwayStart); iter != PathwayMap.end() )

        auto       pathway = Pathways.emplace_back(std::make_unique<Pathway>()).get();
        const bool topDown = blockedDirection == Direction::Up || blockedDirection == Direction::Left;
        if constexpr ( AssignStart ) {
            StartPathway = pathway;
        } //if constexpr ( AssignStart )
        else {
            (topDown ? pathway->Begin : pathway->End) = fromCrossing;
        } //else -> if constexpr ( AssignStart )
        PathwayMap.emplace(pathwayStart, pathway);

        do { //while ( field == '.' )
            Direction neighborDirection = *(directionRangeWithout(blockedDirection) |
                                            std::views::drop_while([&input, &pathwayStart](Direction d) noexcept {
                                                auto p = pathwayStart.moved(d);
                                                return input[p.Row][p.Column] == '#';
                                            })).begin();
            ++pathway->Length;
            pathwayStart.move(neighborDirection);
            blockedDirection = turnAround(neighborDirection);
            field            = input[pathwayStart.Row][pathwayStart.Column];

            if ( pathwayStart == targetCoordinate ) {
                PathwayMap.emplace(pathwayStart, pathway);
                TargetPathway = pathway;
                return TargetPathway;
            } //if ( pathwayStart == targetCoordinate )
        } while ( field == '.' );

        switch ( field ) {
            case '>' :
            case 'v' : {
                Direction toCrossing = fieldToDirection(field);
                if ( !topDown ) {
                    toCrossing = turnAround(toCrossing);
                } //if ( !topDown )
                PathwayMap.emplace(pathwayStart, pathway);
                (topDown ? pathway->End : pathway->Begin) =
                    buildCrossing(input, pathwayStart.moved(toCrossing), toCrossing, pathway, targetCoordinate);
                break;
            } //case '>' & 'v'
            default : throwIfInvalid(false);
        } //switch ( field )
        return pathway;
    }

    Crossing* buildCrossing(const std::vector<std::string_view>& input, const Coordinate crossingCoordinate,
                            Direction intoCrossing, Pathway* firstPathway, const Coordinate& targetCoordinate) {
        auto fromCrossing = turnAround(intoCrossing);
        if ( auto iter = Crossings.find(crossingCoordinate); iter != Crossings.end() ) {
            auto crossing                                              = iter->second.get();
            crossing->Pathways[static_cast<std::size_t>(fromCrossing)] = firstPathway;
            return crossing;
        } //if ( auto iter = Crossings.find(crossingCoordinate); iter != Crossings.end() )

        auto crossing = Crossings.emplace(crossingCoordinate, std::make_unique<Crossing>()).first->second.get();
        crossing->Pathways[static_cast<std::size_t>(fromCrossing)] = firstPathway;
        for ( auto direction : directionRangeWithout(fromCrossing) ) {
            crossing->Pathways[static_cast<std::size_t>(direction)] = buildPathway<false>(
                input, crossingCoordinate.moved(direction), turnAround(direction), crossing, targetCoordinate);
        } //for ( auto direction : directionRangeWithout(fromCrossing) )
        return crossing;
    }

    static Direction fieldToDirection(char field) noexcept {
        switch ( field ) {
            case '>' : return Direction::Right;
            case 'v' : return Direction::Down;
            default  : throwIfInvalid(false); return Direction::Up;
        } //switch ( field )
    }

    static Direction directionOfPathway(Pathway* pathway, Crossing* crossing) noexcept {
        return static_cast<Direction>(
            std::ranges::distance(crossing->Pathways.begin(), std::ranges::find(crossing->Pathways, pathway)));
    }
};
} //namespace

bool challenge23(const std::vector<std::string_view>& input) {
    Graph graph{input};

    auto maxDirectedPathLength = graph.calculateLongestPath<true>();
    myPrint(" == Result of Part 1: {:d} ==\n", maxDirectedPathLength);

    auto maxUndirectedPathLength = graph.calculateLongestPath<false>();
    myPrint(" == Result of Part 2: {:d} ==\n", maxUndirectedPathLength);

    return maxDirectedPathLength == 2106 && maxUndirectedPathLength == 6350;
}

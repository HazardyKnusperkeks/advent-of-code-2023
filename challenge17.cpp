#include "challenge17.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
enum class Direction { Up, Left, Down, Right };

Direction turnLeft(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 4 - 1) % 4);
}

Direction turnRight(Direction d) noexcept {
    return static_cast<Direction>((static_cast<int>(d) + 1) % 4);
}

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

    Coordinate& move(Direction direction) noexcept {
        switch ( direction ) {
            using enum Direction;
            case Left  : --Column; break;
            case Up    : --Row; break;
            case Right : ++Column; break;
            case Down  : ++Row; break;
        } //switch ( direction )
        return *this;
    }

    Coordinate moved(Direction direction) const noexcept {
        auto ret = *this;
        ret.move(direction);
        return ret;
    }
};
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

namespace {
Coordinate Target;

struct Node {
    Coordinate   Pos;
    Direction    LastDirection;
    std::int64_t CurrentHeatLoss;

    int distanceToTarget(void) const noexcept {
        return static_cast<int>(Target.Row - Pos.Row + Target.Column - Pos.Column);
    }
};

struct Heuristic {
    bool operator()(const Node& lhs, const Node& rhs) const noexcept {
       return lhs.CurrentHeatLoss > rhs.CurrentHeatLoss ||
              (lhs.CurrentHeatLoss == rhs.CurrentHeatLoss && lhs.distanceToTarget() > rhs.distanceToTarget());
    }
};

struct Hasher {
    std::size_t operator()(const Node& node) const noexcept {
        std::hash<Coordinate> c;
        std::hash<Direction>  d;
        return (c(node.Pos) << 8) ^ d(node.LastDirection);
    }
};

struct KindOfEquality {
    bool operator()(const Node& lhs, const Node& rhs) const noexcept {
        return lhs.Pos == rhs.Pos && lhs.LastDirection == rhs.LastDirection;
    }
};

struct PathFinder {
    const std::vector<std::vector<std::int8_t>>                    HeatLossMap;
    std::priority_queue<Node, std::vector<Node>, Heuristic>        NodesToVisit;
    std::unordered_map<Node, std::int64_t, Hasher, KindOfEquality> Visited;

    PathFinder(std::vector<std::vector<std::int8_t>> heatLossMap) noexcept : HeatLossMap{std::move(heatLossMap)} {
        Target = {HeatLossMap.size() - 1, HeatLossMap.front().size() - 1};
        return;
    }

    bool isValid(const Coordinate& pos) const noexcept {
        return pos.Row <= Target.Row && pos.Column <= Target.Column;
    }

    std::int64_t findPath(int minPerDirection, int maxPerDirection) noexcept {
        while ( !NodesToVisit.empty() ) {
            NodesToVisit.pop();
        } //while ( !NodesToVisit.empty() )
        Visited.clear();

        NodesToVisit.emplace(Coordinate{0, 1}, Direction::Right, 0);
        NodesToVisit.emplace(Coordinate{1, 0}, Direction::Down, 0);

        std::int64_t ret = std::numeric_limits<std::int64_t>::max();

        while ( !NodesToVisit.empty() ) {
            auto currentNode = NodesToVisit.top();
            NodesToVisit.pop();

            auto iter = Visited.find(currentNode);
            if ( iter != Visited.end() && iter->second <= currentNode.CurrentHeatLoss ) {
                continue;
            } //if ( iter != Visited.end() && iter->second <= currentNode.CurrentHeatLoss )

            if ( iter == Visited.end() ) {
                Visited.insert({currentNode, currentNode.CurrentHeatLoss});
            } //if ( iter == Visited.end() )
            else {
                iter->second = currentNode.CurrentHeatLoss;
            } //else -> if ( iter == Visited.end() )

            for ( int stepsDone = 0; stepsDone < maxPerDirection; ++stepsDone ) {
                if ( !isValid(currentNode.Pos) ) {
                    break;
                } //if ( !isValid(currentNode.Pos) )

                currentNode.CurrentHeatLoss += HeatLossMap[currentNode.Pos.Row][currentNode.Pos.Column];

                if ( currentNode.Pos == Target ) {
                    ret = std::min(ret, currentNode.CurrentHeatLoss);
                    break;
                } //if ( currentNode.Pos == Target )

                if ( stepsDone >= minPerDirection ) {
                    for ( auto dir : {turnLeft(currentNode.LastDirection), turnRight(currentNode.LastDirection)} ) {
                        NodesToVisit.emplace(currentNode.Pos.moved(dir), dir, currentNode.CurrentHeatLoss);
                    } //for ( auto dir : {turnLeft(currentNode.LastDirection), turnRight(currentNode.LastDirection)} )
                } //if ( stepsDone >= minPerDirection )

                currentNode.Pos.move(currentNode.LastDirection);
            } //for ( int stepsDone = 0; stepsDone < maxPerDirection; ++stepsDone )
        } //while ( !NodesToVisit.empty() )
        return ret;
    }
};

auto convert(const std::vector<std::string_view>& input) noexcept {
    std::vector<std::vector<std::int8_t>> ret;
    ret.resize(input.size());
    std::ranges::transform(input, ret.begin(), [](std::string_view line) noexcept {
        std::vector<std::int8_t> lineRet;
        lineRet.resize(line.size());
        std::ranges::transform(line, lineRet.begin(), [](char c) noexcept { return c - '0'; });
        return lineRet;
    });
    return ret;
}
} //namespace

bool challenge17(const std::vector<std::string_view>& input) {
    throwIfInvalid(!input.empty());

    PathFinder   pathFinder{convert(input)};
    std::int64_t min1 = pathFinder.findPath(0, 3);
    myPrint(" == Result of Part 1: {:d} ==\n", min1);

    std::int64_t min2 = pathFinder.findPath(3, 10);
    myPrint(" == Result of Part 2: {:d} ==\n", min2);

    return min1 == 1004 && min2 == 1171;
}

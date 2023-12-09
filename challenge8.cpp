#include "challenge8.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <string_view>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
enum class Direction : bool { Left, Right };

struct Node {
    std::string_view Name;
    std::string_view Left;
    std::string_view Right;
};

struct Map {
    std::vector<Direction>                     Directions;
    std::unordered_map<std::string_view, Node> Nodes;
    std::vector<std::string_view>              GhostStarts;
};

std::vector<Direction> parseDirections(std::string_view input) {
    std::vector<Direction> ret;
    ret.resize(input.size());
    std::ranges::copy(input | std::views::transform([](char c) {
                          if ( c == 'L' ) {
                              return Direction::Left;
                          } //if ( c == 'L' )
                          throwIfInvalid(c == 'R');
                          return Direction::Right;
                      }),
                      ret.begin());
    return ret;
}

Map parse(const std::vector<std::string_view>& input) {
    Map map;

    throwIfInvalid(input.size() >= 1);
    map.Directions = parseDirections(input[0]);

    for ( auto line : input | std::views::drop(1) ) {
        const auto match = ctre::match<"(\\w{3}) = \\((\\w{3}), (\\w{3})\\)">(line);
        const std::string_view name  = match.get<1>();
        const std::string_view left  = match.get<2>();
        const std::string_view right = match.get<3>();

        map.Nodes.emplace(std::piecewise_construct, std::tuple{name}, std::tuple{name, left, right});
        if ( name.ends_with('A') ) {
            map.GhostStarts.push_back(name);
        } //if ( name.ends_with('A') )
    } //for ( auto line : input | std::views::drop(1) )

    return map;
}

template<typename F>
std::int64_t calcSteps(const Map& map, std::string_view start, F atTarget) {
    std::int64_t steps         = 0;
    auto         current       = start;
    auto         nextDirection = map.Directions.begin();

    const auto moveDirection   = [directionEnd = map.Directions.end(),
                                begin        = map.Directions.begin()](auto direction) noexcept {
        if ( ++direction == directionEnd ) {
            direction = begin;
        } //if ( ++direction == directionEnd )
        return direction;
    };

    const auto next = [nodeEnd = map.Nodes.end(), &map, &nextDirection](std::string_view node) {
        auto mapNode = map.Nodes.find(node);
        throwIfInvalid(mapNode != nodeEnd);
        return *nextDirection == Direction::Left ? mapNode->second.Left : mapNode->second.Right;
    };

    while ( !atTarget(current) ) {
        ++steps;
        current       = next(current);
        nextDirection = moveDirection(nextDirection);
    } //while ( !atTarget(current) )
    return steps;
}
} //namespace

bool challenge8(const std::vector<std::string_view> &input) {
    const auto map   = parse(input);
    const auto steps = calcSteps(map, "AAA"sv, [](std::string_view node) noexcept { return node == "ZZZ"sv; });
    myPrint(" == Result of Part 1: {:d} ==\n", steps);

    std::vector<std::int64_t> ghostSteps(map.GhostStarts.size());
    std::ranges::transform(map.GhostStarts, ghostSteps.begin(), [&map](std::string_view start) {
        return calcSteps(map, start, [](std::string_view node) noexcept { return node.ends_with('Z'); });
    });

    const auto ghostStep = std::ranges::fold_left(ghostSteps, 1, std::lcm<std::int64_t, std::int64_t>);
    myPrint(" == Result of Part 2: {:d} ==\n", ghostStep);

    return steps == 12083 && ghostStep == 13'385'272'668'829;
}

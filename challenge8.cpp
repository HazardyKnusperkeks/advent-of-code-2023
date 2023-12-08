#include "challenge8.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
void throwIfInvalid(bool valid, const char* msg = "Invalid Data") {
    if ( !valid ) {
        throw std::runtime_error{msg};
    } //if ( !valid )
    return;
}

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

Map parse(const std::vector<std::string>& input) {
    enum class State { Directions, Node, Equal, Left, Right } state = State::Directions;
    Map   map;
    Node* currentNode;

    for ( std::string_view word : input ) {
        switch ( state ) {
            case State::Directions : {
                map.Directions = parseDirections(word);
                state          = State::Node;
                break;
            } //case State::Directions

            case State::Node : {
                currentNode       = &map.Nodes.emplace(word, Node{}).first->second;
                currentNode->Name = word;
                if ( word.ends_with('A') ) {
                    map.GhostStarts.push_back(word);
                } //if ( word.ends_with('A') )
                state = State::Equal;
                break;
            } //case State::Node

            case State::Equal : {
                throwIfInvalid(word == "="sv);
                state = State::Left;
                break;
            } //case State::Equal

            case State::Left : {
                throwIfInvalid(word.size() == 5);
                throwIfInvalid(word[0] == '(');
                throwIfInvalid(word[4] == ',');
                currentNode->Left = word.substr(1, 3);
                state             = State::Right;
                break;
            } //case State::Left

            case State::Right : {
                throwIfInvalid(word.size() == 4);
                throwIfInvalid(word[3] == ')');
                currentNode->Right = word.substr(0, 3);
                state              = State::Node;
                break;
            } //case State::Right
        } //switch ( state )
    } //for ( const auto& word : input )

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

void challenge8(const std::vector<std::string>& input) {
    std::cout << " == Starting Challenge 8 ==\n";

    const auto map   = parse(input);
    const auto steps = calcSteps(map, "AAA"sv, [](std::string_view node) noexcept { return node == "ZZZ"sv; });

    std::cout << " == Result of Challenge 8 Part 1: " << steps << " ==\n";

    std::vector<std::int64_t> ghostSteps(map.GhostStarts.size());
    std::ranges::transform(map.GhostStarts, ghostSteps.begin(), [&map](std::string_view start) {
        return calcSteps(map, start, [](std::string_view node) noexcept { return node.ends_with('Z'); });
    });

    const auto ghostStep = std::ranges::fold_left(ghostSteps, 1, std::lcm<std::int64_t, std::int64_t>);

    std::cout << " == Result of Challenge 8 Part 2: " << ghostStep << " ==\n";
    return;
}

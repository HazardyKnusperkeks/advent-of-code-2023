#include "challenge5.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <future>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>

using namespace std::string_view_literals;

namespace {
struct Map {
    std::int64_t Source;
    std::int64_t Destination;
    std::int64_t Length;
};

struct SeedMap {
    std::vector<std::int64_t>     Seeds;
    std::vector<std::vector<Map>> Maps;
};

void throwIfInvalid(bool valid) {
    if ( !valid ) {
        throw std::runtime_error{"Invalid Data"};
    } //if ( !valid )
    return;
}

std::optional<std::int64_t> convert(std::string_view input) {
    if ( !std::isdigit(input[0]) ) {
        return std::nullopt;
    } //if ( !std::isdigit(input[0]) )

    std::int64_t ret;
    auto         result = std::from_chars(input.begin(), input.end(), ret);
    throwIfInvalid(result.ec == std::errc{});
    return result.ptr == input.data() ? std::nullopt : std::optional{ret};
}

SeedMap parse(const std::vector<std::string>& input) {
    enum class State { Seeds, Seed, Map, DestinationStart, SourceStart, Length } state = State::Seeds;
    SeedMap           ret;
    std::vector<Map>* currentMaps;
    Map*              currentMap;

    for ( const auto& word : input ) {
        switch ( state ) {
            case State::Seeds : {
                throwIfInvalid(word == "seeds:"sv);
                state = State::Seed;
                break;
            } //case State::Seeds

            case State::Seed : {
                auto number = convert(word);
                if ( !number ) {
                    state = State::Map;
                    break;
                } //if ( !number )

                ret.Seeds.push_back(*number);
                break;
            } //case State::Seed

            case State::Map : {
                throwIfInvalid(word == "map:"sv);
                currentMaps = &ret.Maps.emplace_back();
                state       = State::DestinationStart;
                break;
            } //case State::Map

            case State::DestinationStart : {
                auto number = convert(word);
                if ( !number ) {
                    state = State::Map;
                    break;
                } //if ( !number )
                currentMap              = &currentMaps->emplace_back();
                currentMap->Destination = *number;
                state                   = State::SourceStart;
                break;
            } //case State::DesinationStart

            case State::SourceStart : {
                auto number = convert(word);
                throwIfInvalid(number.has_value());
                currentMap->Source = *number;
                state              = State::Length;
                break;
            } //case State::SourceStart

            case State::Length : {
                auto number = convert(word);
                throwIfInvalid(number.has_value());
                currentMap->Length = *number;
                state              = State::DestinationStart;
                break;
            } //case State::Length
        } //switch ( state )
    } //for ( const auto& word : input )

    return ret;
}
} //namespace

void challenge5(const std::vector<std::string>& input) {
    std::cout << " == Starting Challenge 5 ==\n";

    const auto seedMap = parse(input);
    auto       work    = seedMap.Seeds;

    auto applyMaps     = [&seedMap](std::int64_t entry) noexcept {
        for ( const auto& maps : seedMap.Maps ) {
            for ( const auto& map : maps ) {
                if ( entry >= map.Source && entry < map.Source + map.Length ) {
                    entry -= map.Source;
                    entry += map.Destination;
                    break;
                } //if ( entry >= map.Source && entry < map.Source + map.Length )
            } //for ( const auto& map : maps )
        } //for ( const auto& maps : seedMap.Maps )
        return entry;
    };

    std::ranges::for_each(seedMap.Seeds, applyMaps);
    auto min = std::ranges::min(seedMap.Seeds | std::views::transform(applyMaps));
    std::cout << " == Result of Challenge 5 Part 1: " << min << " ==\n";

    work.clear();
    min = std::numeric_limits<std::int64_t>::max();
    throwIfInvalid(seedMap.Seeds.size() % 2 == 0);

    std::vector<std::future<std::int64_t>> results;

    for ( std::size_t i = 0; i < seedMap.Seeds.size(); i += 2 ) {
        results.push_back(std::async(std::launch::async, [i, &seedMap, &applyMaps](void) noexcept {
            return std::ranges::min(std::views::iota(seedMap.Seeds[i]) | std::views::take(seedMap.Seeds[i + 1]) |
                                    std::views::transform(applyMaps));
        }));
    } //for ( std::size_t i = 0; i < seedMap.Seeds.size(); i += 2 )

    min = std::ranges::min(results | std::views::transform([](auto& f) noexcept { return f.get(); }));
    std::cout << " == Result of Challenge 5 Part 2: " << min << " ==\n";
    return;
}

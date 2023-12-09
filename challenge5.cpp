#include "challenge5.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <future>
#include <ranges>
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

SeedMap parse(const std::vector<std::string_view>& input) {
    enum class State { Seeds, Seed, Map, DestinationStart, SourceStart, Length } state = State::Seeds;
    SeedMap           ret;
    std::vector<Map>* currentMaps = nullptr;
    Map*              currentMap;

    for ( auto line : input ) {
        for ( const auto& word : splitString(line, ' ') ) {
            switch ( state ) {
                case State::Seeds : {
                    throwIfInvalid(word == "seeds:"sv);
                    state = State::Seed;
                    break;
                } //case State::Seeds

                case State::Seed : {
                    auto number = convertOptionally(word);
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
                    auto number = convertOptionally(word);
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
                    auto number = convertOptionally(word);
                    throwIfInvalid(number.has_value());
                    currentMap->Source = *number;
                    state              = State::Length;
                    break;
                } //case State::SourceStart

                case State::Length : {
                    auto number = convertOptionally(word);
                    throwIfInvalid(number.has_value());
                    currentMap->Length = *number;
                    state              = State::DestinationStart;
                    break;
                } //case State::Length
            } //switch ( state )
        } //for ( const auto& word : splitString(line, ' ') )
    } //for ( auto line : input )

    return ret;
}
} //namespace

bool challenge5(const std::vector<std::string_view>& input) {
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
    auto min1 = std::ranges::min(seedMap.Seeds | std::views::transform(applyMaps));
    myPrint(" == Result of Part 1: {:d} ==\n", min1);

    work.clear();
    throwIfInvalid(seedMap.Seeds.size() % 2 == 0);

    std::vector<std::future<std::int64_t>> results;

    for ( std::size_t i = 0; i < seedMap.Seeds.size(); i += 2 ) {
        results.push_back(std::async(std::launch::async, [i, &seedMap, &applyMaps](void) noexcept {
            return std::ranges::min(std::views::iota(seedMap.Seeds[i]) | std::views::take(seedMap.Seeds[i + 1]) |
                                    std::views::transform(applyMaps));
        }));
    } //for ( std::size_t i = 0; i < seedMap.Seeds.size(); i += 2 )

    auto min2 = std::ranges::min(results | std::views::transform([](auto& f) noexcept { return f.get(); }));
    myPrint(" == Result of Part 1: {:d} ==\n", min2);

    return min1 == 51752125 && min2 == 12634632;
}

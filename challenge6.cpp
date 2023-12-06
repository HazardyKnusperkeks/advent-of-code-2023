#include "challenge6.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string_view>

using namespace std::string_view_literals;

namespace {
struct Race {
    std::int64_t Time;
    std::int64_t Distance;
};

void throwIfInvalid(bool valid) {
    if ( !valid ) {
        throw std::runtime_error{"Invalid Data"};
    } //if ( !valid )
    return;
}

std::int64_t convert(std::string_view input) {
    std::int64_t ret;
    throwIfInvalid(std::from_chars(input.begin(), input.end(), ret).ec == std::errc{});
    return ret;
}

std::pair<std::vector<Race>, Race> parse(const std::vector<std::string>& input) {
    enum class State { Time, Times, Distances } state = State::Time;
    std::vector<Race> races;
    Race*             currentRace;
    std::string       overallTime;
    std::string       overallDuration;

    for ( const auto& word : input ) {
        switch ( state ) {
            case State::Time : {
                throwIfInvalid(word == "Time:"sv);
                state = State::Times;
                break;
            } //case State::Time

            case State::Times : {
                if ( word == "Distance:"sv ) {
                    throwIfInvalid(!races.empty());
                    state       = State::Distances;
                    currentRace = &races[0];
                    break;
                } //if ( word == "Distance:"sv )

                overallTime += word;
                races.push_back({convert(word), 0});
                break;
            } //case State::Times

            case State::Distances : {
                throwIfInvalid(currentRace - &races[0] < static_cast<std::int64_t>(races.size()));
                overallDuration       += word;
                currentRace->Distance  = convert(word);
                ++currentRace;
                break;
            } //case State::Distances
        } //switch ( state )
    } //for ( const auto& word : input )

    return {races, Race{convert(overallTime), convert(overallDuration)}};
}

std::int64_t numberOfWinStrategies(const Race& race) noexcept {
    std::int64_t ret = 0;
    std::ranges::for_each(std::views::iota(1, race.Time), [&race, &ret](std::int64_t holdDown) noexcept {
        auto timeToDrive     = race.Time - holdDown;
        auto distanceReached = timeToDrive * holdDown;
        if ( distanceReached > race.Distance ) {
            ++ret;
        } //if ( distanceReached > race.Distance )
        return;
    });
    return ret;
}
} //namespace

void challenge6(const std::vector<std::string>& input) {
    std::cout << " == Starting Challenge 6 ==\n";

    const auto [races, bigRace] = parse(input);

    const auto product =
        std::ranges::fold_left(races | std::views::transform(numberOfWinStrategies), 1, std::multiplies<>{});
    std::cout << " == Result of Challenge 6 Part 1: " << product << " ==\n";

    const auto wins = numberOfWinStrategies(bigRace);

    std::cout << " == Result of Challenge 6 Part 2: " << wins << " ==\n";
    return;
}

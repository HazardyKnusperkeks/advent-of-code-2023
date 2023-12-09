#include "challenge6.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

namespace {
struct Race {
    std::int64_t Time;
    std::int64_t Distance;
};

std::pair<std::vector<Race>, Race> parse(const std::vector<std::string_view>& input) {
    enum class State { Time, Times, Distances } state = State::Time;
    std::vector<Race> races;
    Race*             currentRace;
    std::string       overallTime;
    std::string       overallDuration;

    for ( auto line : input ) {
        for ( const auto& word : splitString(line, ' ') ) {
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
        } //for ( const auto& word : splitString(line, ' ') )
    } //for ( auto line : input )

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

bool challenge6(const std::vector<std::string_view> &input) {
    const auto [races, bigRace] = parse(input);

    const auto product =
        std::ranges::fold_left(races | std::views::transform(numberOfWinStrategies), 1, std::multiplies<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", product);

    const auto wins = numberOfWinStrategies(bigRace);
    myPrint(" == Result of Part 2: {:d} ==\n", wins);

    return product == 1710720 && wins == 35349468;
}

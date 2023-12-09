#include "challenge2.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>

using namespace std::string_view_literals;

namespace {
struct Game {
    std::int64_t Number   = 0;
    std::int64_t MaxRed   = 0;
    std::int64_t MaxBlue  = 0;
    std::int64_t MaxGreen = 0;
};

std::vector<Game> parse(const std::vector<std::string_view>& input) {
    enum class State { Game, Number, Count, Color } state = State::Game;
    std::vector<Game> ret;
    Game*             currentGame;
    std::int64_t      currentCount;

    for ( auto line : input ) {
        for ( const auto& word : splitString(line, ' ') ) {
            switch ( state ) {
                case State::Game : {
                    throwIfInvalid(word == "Game"sv);
                    currentGame = &ret.emplace_back();
                    state       = State::Number;
                    break;
                } //case State::Game

                case State::Number : {
                    currentGame->Number = convert(word);
                    state               = State::Count;
                    break;
                } //case State::Number

                case State::Count : {
                    currentCount = convert(word);
                    state        = State::Color;
                    break;
                } //case State::Count

                case State::Color : {
                    std::string_view colorString = word;
                    switch ( word.back() ) {
                        case ';' :
                        case ',' : {
                            colorString.remove_suffix(1);
                            state = State::Count;
                            break;
                        } //case ';' & ','

                        default : {
                            state = State::Game;
                            break;
                        } //default
                    } //switch ( word.back() )

                    auto checkAndAssign = [&colorString, &currentCount](std::string_view check,
                                                                        std::int64_t&    value) noexcept {
                        if ( colorString != check ) {
                            return false;
                        } //if ( colorString != check )

                        value = std::max(value, currentCount);
                        return true;
                    };
                    throwIfInvalid(checkAndAssign("red"sv, currentGame->MaxRed) ||
                                   checkAndAssign("blue"sv, currentGame->MaxBlue) ||
                                   checkAndAssign("green"sv, currentGame->MaxGreen));
                    break;
                } //case State::Color
            } //switch ( state )
        } //for ( const auto& word : input )
    } //for ( auto line : input )
    return ret;
}

bool validFilter(const Game& game) noexcept {
    return game.MaxRed <= 12 && game.MaxBlue <= 14 && game.MaxGreen <= 13;
}

auto calcPower(const Game& game) noexcept {
    return game.MaxRed * game.MaxBlue * game.MaxGreen;
}
} //namespace

bool challenge2(const std::vector<std::string_view> &input) {
    const std::vector<Game> games = parse(input);

    auto       possibleGames      = games | std::views::filter(validFilter);
    auto       possibleIds        = possibleGames | std::views::transform(&Game::Number);
    const auto sum1               = std::ranges::fold_left(possibleIds, 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto       powers = games | std::views::transform(calcPower);
    const auto sum2   = std::ranges::fold_left(powers, 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 2006 && sum2 == 84911;
}

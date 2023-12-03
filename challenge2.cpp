#include "challenge2.hpp"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string_view>

using namespace std::string_view_literals;

namespace {
struct Game {
    int Number   = 0;
    int MaxRed   = 0;
    int MaxBlue  = 0;
    int MaxGreen = 0;
};

void throwIfInvalid(bool valid) {
    if ( !valid ) {
        throw std::runtime_error{"Invalid Data"};
    } //if ( !valid )
    return;
}

int convert(std::string_view input) {
    int ret;
    throwIfInvalid(std::from_chars(input.begin(), input.end(), ret).ec == std::errc{});
    return ret;
}

std::vector<Game> parse(const std::vector<std::string>& input) {
    enum class State { Game, Number, Count, Color } state = State::Game;
    std::vector<Game> ret;
    Game*             currentGame;
    int               currentCount;

    for ( const auto& word : input ) {
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

                auto checkAndAssign = [&colorString, &currentCount](std::string_view check, int& value) noexcept {
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

    return ret;
}

bool validFilter(const Game& game) noexcept {
    return game.MaxRed <= 12 && game.MaxBlue <= 14 && game.MaxGreen <= 13;
}

int calcPower(const Game& game) noexcept {
    return game.MaxRed * game.MaxBlue * game.MaxGreen;
}
} //namespace

void challenge2(const std::vector<std::string>& input) {
    std::cout << " == Starting Challenge 2 ==\n";

    const std::vector<Game> games = parse(input);

    auto       possibleGames      = games | std::views::filter(validFilter);
    auto       possibleIds        = possibleGames | std::views::transform(&Game::Number);
    const auto sum1               = std::ranges::fold_left(possibleIds, 0, std::plus<>{});
    std::cout << " == Result of Challenge 2 Part 1: " << sum1 << " ==\n";

    auto       powers = games | std::views::transform(calcPower);
    const auto sum2   = std::ranges::fold_left(powers, 0, std::plus<>{});

    std::cout << " == Result of Challenge 2 Part 2: " << sum2 << " ==\n";
    return;
}

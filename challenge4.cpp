#include "challenge4.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

namespace {
struct Card {
    std::int64_t              Number = 0;
    std::vector<std::int64_t> WinninNumbers;
    std::vector<std::int64_t> Numbers;
    std::int64_t              Matching = 0;
    std::int64_t              Points   = 0;
};

struct CountingIterator {
    using difference_type = int;

    std::int64_t Count    = 0;
    std::int64_t Dummy;

    std::int64_t& operator*(void) noexcept {
        ++Count;
        return Dummy;
    }

    CountingIterator& operator++(void) noexcept {
        return *this;
    }

    CountingIterator& operator++(int) noexcept {
        return *this;
    }
};

std::vector<Card> parse(const std::vector<std::string_view>& input) {
    enum class State { Card, Number, Winning, Numbers } state = State::Card;
    std::vector<Card> ret;
    Card*             currentCard;

    for ( auto line : input ) {
        for ( const auto& word : splitString(line, ' ') ) {
            switch ( state ) {
                case State::Number : {
                    currentCard->Number = convert(word);
                    state               = State::Winning;
                    break;
                } //case State::Number

                case State::Winning : {
                    if ( word == "|"sv ) {
                        state = State::Numbers;
                        break;
                    } //if ( word == "|"sv )

                    currentCard->WinninNumbers.push_back(convert(word));
                    break;
                } //case State::Winning

                case State::Numbers : {
                    if ( word != "Card"sv ) {
                        currentCard->Numbers.push_back(convert(word));
                        break;
                    } //if ( word != "Card"sv )
                    [[fallthrough]];
                } //case State::Numbers

                case State::Card : {
                    throwIfInvalid(word == "Card"sv);
                    currentCard = &ret.emplace_back();
                    state       = State::Number;
                    break;
                } //case State::Card
            } //switch ( state )
        } //for ( const auto& word : splitString(line, ' ') )
    } //for ( auto line : input )
    return ret;
}

void sortAndCalcPoints(Card& card) noexcept {
    std::ranges::sort(card.WinninNumbers);
    std::ranges::sort(card.Numbers);

    card.Matching = std::ranges::set_intersection(card.WinninNumbers, card.Numbers, CountingIterator{}).out.Count;
    card.Points   = card.Matching == 0 ? 0 : 1 << (card.Matching - 1);

    return;
}
} //namespace

bool challenge4(const std::vector<std::string_view>& input) {
    std::cout << " == Starting Challenge 4 ==\n";

    std::vector<Card> cards = parse(input);
    std::ranges::for_each(cards, sortAndCalcPoints);

    const auto sum1 = std::ranges::fold_left(cards | std::views::transform(&Card::Points), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::vector<int> totalCards;
    totalCards.resize(cards.size());

    for ( std::size_t i = 0, end = cards.size(); i < end; ++i ) {
        const auto& currentCard      = cards[i];
        auto&       currentCardCount = totalCards[i];
        ++currentCardCount;
        const auto cardsToAdd = static_cast<std::size_t>(currentCard.Matching);

        for ( std::size_t j = i + 1, addEnd = std::min(end, i + 1 + cardsToAdd); j < addEnd; ++j ) {
            totalCards[j] += currentCardCount;
        } //for ( std::size_t j = i + 1, addEnd = std::min(end, i + 1 + cardsToAdd); j < addEnd; ++j )
    } //for ( std::size_t i = 0, end = cards.size(); i < end; ++i )

    const auto sum2 = std::ranges::fold_left(totalCards, 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 21568 && sum2 == 11827296;
}

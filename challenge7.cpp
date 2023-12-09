#include "challenge7.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <functional>
#include <ranges>
#include <utility>

using namespace std::string_view_literals;

namespace {
enum KindType { HighCard, OnePair, TwoPair, Three, FullHouse, Four, Five };

constexpr int convertToValue(char c) {
    switch ( c ) {
        case 'T' : return 10;
        case 'J' : return 11;
        case 'Q' : return 12;
        case 'K' : return 13;
        case 'A' : return 14;

        default  : {
            if ( std::isdigit(c) ) {
                return c - '0';
            } //if ( std::isdigit(c) )
            break;
        } //default
    } //switch ( c )

    throwIfInvalid(false);
    return 0; //Compiler sieht nicht, dass es immer throw macht.
}

template<bool WithJoker>
KindType calculateKind(std::array<int, 5>& cards) noexcept {
    constexpr auto      jokerValue = convertToValue('J');
    std::array<int, 15> counter    = {};
    for ( auto& card : cards ) {
        ++counter[static_cast<std::size_t>(card)];
        if constexpr ( WithJoker ) {
            if ( card == jokerValue ) {
                card = 1;
            } //if ( card == jokerValue )
        } //if constexpr ( WithJoker )
    } //for ( auto& card : cards )

    const auto jokers = [&](void) noexcept {
        if constexpr ( WithJoker ) {
            return std::exchange(counter[jokerValue], 0);
        }
        else {
            return 0;
        }
    }();

    std::ranges::sort(counter, std::ranges::greater{});
    switch ( counter[0] ) {
        case 5 : return Five;
        case 4 : return jokers == 1 ? Five : Four;

        case 3 : {
            if constexpr ( WithJoker ) {
                switch ( jokers ) {
                    case 2 : return Five;
                    case 1 : return Four;
                } //switch ( jokers )
            } //if constexpr ( WithJoker )

            return counter[1] == 2 ? FullHouse : Three;
        } //case 3

        case 2 : {
            if constexpr ( WithJoker ) {
                switch ( jokers ) {
                    case 3 : return Five;
                    case 2 : return Four;
                    case 1 : return counter[1] == 2 ? FullHouse : Three;
                } //switch ( jokers )
            } //if constexpr ( WithJoker )

            return counter[1] == 2 ? TwoPair : OnePair;
        } //case 2

        case 1 : {
            if constexpr ( WithJoker ) {
                switch ( jokers ) {
                    case 4 : return Five;
                    case 3 : return Four;
                    case 2 : return Three;
                    case 1 : return OnePair;
                } //switch ( jokers )
            } //if constexpr ( WithJoker )

            break;
        } //case 1

        case 0 : {
            if constexpr ( WithJoker ) {
                switch ( jokers ) {
                    case 5 : return Five;
                    case 4 : return Four;
                    case 3 : return Three;
                    case 2 : return OnePair;
                } //switch ( jokers )
            } //if constexpr ( WithJoker )

            break;
        } //case 0
    } //switch ( counter[0] )

    return HighCard;
}

std::array<int, 5> calculateCards(std::string_view word) {
    throwIfInvalid(word.size() == 5);
    std::array<int, 5> cards;
    for ( std::size_t i = 0; i < 5; ++i ) {
        cards[i] = convertToValue(word[i]);
    } //for ( std::size_t i = 0; i < 5; ++i )
    return cards;
}

std::int64_t convert(std::string_view input) {
    std::int64_t ret;
    throwIfInvalid(std::from_chars(input.begin(), input.end(), ret).ec == std::errc{});
    return ret;
}

struct Hand {
    std::array<int, 5> Cards;
    KindType           Kind;
    std::int64_t       Bid;

    constexpr bool operator<(const Hand& that) const noexcept {
        if ( Kind == that.Kind ) {
            return Cards < that.Cards;
        } //if ( Kind == that.Kind )
        return Kind < that.Kind;
    }

    constexpr auto operator<=>(const Hand& that) const noexcept = default;
};

std::vector<Hand> parse(const std::vector<std::string_view>& input) {
    std::vector<Hand> hands;

    for ( auto line : input ) {
        auto& currentHand = hands.emplace_back();
        auto  splitPos    = line.find(' ');
        currentHand.Cards = calculateCards(line.substr(0, splitPos));
        currentHand.Kind  = calculateKind<false>(currentHand.Cards);
        currentHand.Bid   = convert(line.substr(splitPos + 1));
    } //for ( auto line : input )

    return hands;
}
} //namespace

bool challenge7(const std::vector<std::string_view> &input) {

    auto hands = parse(input);
    std::ranges::sort(hands);
    const auto sum1 = std::ranges::fold_left(
        hands | std::views::transform([rank = 0](const Hand& hand) mutable noexcept { return ++rank * hand.Bid; }), 0,
        std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::ranges::for_each(hands, [](Hand& hand) noexcept {
        hand.Kind = calculateKind<true>(hand.Cards);
        return;
    });
    std::ranges::sort(hands);
    const auto sum2 = std::ranges::fold_left(
        hands | std::views::transform([rank = 0](const Hand& hand) mutable noexcept { return ++rank * hand.Bid; }), 0,
        std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 248'396'258 && sum2 == 246'436'046;
}

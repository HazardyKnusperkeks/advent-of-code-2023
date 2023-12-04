#include "challenge3.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace {
struct Pos {
    std::size_t Row;
    std::size_t Column;

    constexpr bool operator==(const Pos&) const noexcept = default;
};

struct AdjacentSymbolPositions {
    using V = std::vector<Pos>;
    using I = V::const_iterator;

    const I   Start;
    const I   End;
    const Pos MaxDimension;

    struct iterator {
        I         Current;
        I         End;
        const Pos MaxDimension;
        int       Neighbor = 0;

        iterator(I current, I end, Pos maxDimension) noexcept : Current{current}, End{end}, MaxDimension{maxDimension} {
            if ( Current != End ) {
                moveToNextValid();
            } //if ( Current != End )
            return;
        }

        iterator& operator++(void) noexcept {
            if ( ++Neighbor == 9 ) {
                Neighbor = 0;

                if ( ++Current == End ) {
                    return *this;
                } //if ( ++Current == End )
            } //if ( ++Neighbor == 9 )

            moveToNextValid();
            return *this;
        }

        void moveToNextValid(void) noexcept {
            const Pos& pos = *Current;
            switch ( Neighbor ) {
                case 0 :
                case 1 :
                case 2 : {
                    if ( pos.Row == 0 ) {
                        Neighbor = 4;
                    } //if ( pos.Row == 0 )
                    break;
                } //case top row

                case 4 : {
                    ++Neighbor;
                    break;
                } //case 4 (the position )

                case 6 :
                case 7 :
                case 8 : {
                    if ( pos.Row == MaxDimension.Row ) {
                        Neighbor = 0;
                        if ( ++Current == End ) {
                            return;
                        } //if ( if ( ++Current == End ) )
                        moveToNextValid();
                        return;
                    } //if ( pos.Row == MaxDimension.Row )
                } //case bottom row
            } //switch ( Neighbor )

            switch ( Neighbor ) {
                case 0 :
                case 6 : {
                    if ( pos.Column == 0 ) {
                        ++Neighbor;
                    } //if ( pos.Column == 0 )
                    break;
                } //case left column

                case 3 : {
                    if ( pos.Column != 0 ) {
                        break;
                    } //if ( pos.Column != 0 )
                    Neighbor = 6;
                    [[fallthrough]];
                } //case 3 (center left column)

                case 2 :
                case 5 : {
                    if ( pos.Column == MaxDimension.Column ) {
                        ++Neighbor;
                        moveToNextValid();
                        return;
                    } //if ( pos.Column == MaxDimension.Column )
                    break;
                } //case right column

                case 8 : {
                    if ( pos.Column == MaxDimension.Column ) {
                        Neighbor = 0;
                        if ( ++Current == End ) {
                            return;
                        } //if ( if ( ++Current == End ) )
                        moveToNextValid();
                        return;
                    } //if ( pos.Column == MaxDimension.Column )
                    break;
                } //case 9 (bottom right)
            } //switch ( Neighbor )

            return;
        }

        Pos operator*(void) const noexcept {
            Pos ret = *Current;
            switch ( Neighbor ) {
                case 0 :
                case 1 :
                case 2 : --ret.Row; break;

                case 6 :
                case 7 :
                case 8 : ++ret.Row; break;
            } //switch ( Neighbor )

            switch ( Neighbor ) {
                case 0 :
                case 3 :
                case 6 : --ret.Column; break;

                case 2 :
                case 5 :
                case 8 : ++ret.Column; break;
            } //switch ( Neighbor )

            return ret;
        }

        constexpr bool operator==(const iterator&) const noexcept = default;
    };

    AdjacentSymbolPositions(const V& positions, Pos maxDimension) noexcept :
            Start(positions.begin()), End(positions.end()), MaxDimension{maxDimension} {
        return;
    }

    AdjacentSymbolPositions(I start, I end, Pos maxDimension) noexcept :
            Start(start), End(end), MaxDimension{maxDimension} {
        return;
    }

    iterator begin(void) const noexcept {
        return {Start, End, MaxDimension};
    }

    iterator end(void) const noexcept {
        return {End, End, MaxDimension};
    }
};

auto extractNumber(const std::string& str, std::size_t pos) {
    int        number;
    const auto start  = &str[pos];
    const auto end    = start + str.size() - pos;
    const auto result = std::from_chars(start, end, number);
    if ( result.ec != std::errc{} ) {
        throw std::runtime_error{"Invalid?"};
    } //if ( result.ec != std::errc{} )
    return std::pair{number, static_cast<std::size_t>(result.ptr - str.data())};
}
} //namespace

namespace std {
template<>
struct hash<Pos> {
    size_t operator()(const Pos& p) const noexcept {
        std::hash<std::size_t> h;
        return h(p.Row << 8) ^ h(p.Column);
    }
};
} //namespace std

void challenge3(const std::vector<std::string>& input) {
    std::cout << " == Starting Challenge 3 ==\n";

    std::vector<Pos>             symbolPositions;
    std::vector<Pos>             gearPositions;
    std::unordered_map<Pos, int> numbers;
    std::unordered_map<Pos, Pos> additionalNumberPositions;
    std::size_t                  maxColumn = 0;

    for ( std::size_t row = 0; row < input.size(); ++row ) {
        const auto& rowText = input[row];
        std::size_t pos     = rowText.find_first_not_of('.');
        maxColumn           = std::max(maxColumn, rowText.size());

        while ( pos != std::string::npos ) {
            if ( std::isdigit(static_cast<unsigned char>(rowText[pos])) ) {
                auto [number, endOfNumber] = extractNumber(rowText, pos);
                numbers.emplace(Pos{row, pos}, number);
                for ( auto numberLength = endOfNumber - pos; numberLength > 1; --numberLength ) {
                    additionalNumberPositions.emplace(Pos{row, pos + numberLength - 1}, Pos{row, pos});
                } //for ( auto numberLength = endOfNumber - pos; numberLength > 1; --numberLength )
                pos = endOfNumber;
            } //if ( std::isdigit(static_cast<unsigned char>(rowText[pos])) )
            else {
                symbolPositions.push_back({row, pos});
                if ( rowText[pos] == '*' ) {
                    gearPositions.push_back({row, pos});
                } //if ( rowText[pos] == '*' )
                ++pos;
            } //else -> if ( std::isdigit(static_cast<unsigned char>(rowText[pos])) )
            pos = rowText.find_first_not_of('.', pos);
        } //while ( pos != std::string::npos )
    } //for ( std::size_t row = 0; row < input.size(); ++row )

    const Pos maxDimensions{input.size(), maxColumn};

    int  sum2        = 0;
    auto gearNumbers = numbers;

    for ( auto gear = gearPositions.begin(); gear != gearPositions.end(); ++gear ) {
        AdjacentSymbolPositions positionsForThatGear{gear, std::next(gear), maxDimensions};
        int                     counter = 0;
        int                     power   = 1;

        for ( auto position : positionsForThatGear ) {
            if ( auto mappingIter = additionalNumberPositions.find(position);
                 mappingIter != additionalNumberPositions.end() ) {
                position = mappingIter->second;
            } //if ( additionalNumberPositions.find(position) != additionalNumberPositions.end() )

            if ( auto iter = gearNumbers.find(position); iter != gearNumbers.end() ) {
                if ( ++counter == 3 ) {
                    break;
                } //if ( ++counter == 3 )
                power *= iter->second;
                gearNumbers.erase(iter);
            } //if ( auto iter = gearNumbers.find(position); iter != gearNumbers.end() )
        } //for ( auto position : positionsForThatGear )

        if ( counter == 2 ) {
            sum2 += power;
        } //if ( counter == 2 )
    } //for ( auto gear = gearPositions.begin(); gear != gearPositions.end(); ++gear )

    AdjacentSymbolPositions adjacentSymbolPositions{symbolPositions, maxDimensions};

    int sum1 = 0;
    for ( auto position : adjacentSymbolPositions ) {
        if ( auto mappingIter = additionalNumberPositions.find(position);
             mappingIter != additionalNumberPositions.end() ) {
            position = mappingIter->second;
            additionalNumberPositions.erase(mappingIter);
        } //if ( additionalNumberPositions.find(position) != additionalNumberPositions.end() )

        if ( auto iter = numbers.find(position); iter != numbers.end() ) {
            sum1 += iter->second;
            numbers.erase(iter);
        } //if ( auto iter = numbers.find(position); iter != Numbers.end() )
    } //for ( auto position : adjacentSymbolPositions )

    std::cout << " == Result of Challenge 3 Part 1: " << sum1 << " ==\n";
    std::cout << " == Result of Challenge 3 Part 2: " << sum2 << " ==\n";

    return;
}

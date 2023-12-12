#include "challenge12.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <unordered_map>

namespace {
enum SpringInfo : char { Damaged = '#', Operational = '.', Unknown = '?' };

struct Line {
    std::string               Springs;
    std::vector<std::int64_t> DamagedGroups;
    std::int64_t              NumberOfUnknown;
    std::int64_t              NumberOfDamagedInList;
    std::int64_t              NumberOfDamagedInGroups;

    bool operator==(const Line& that) const noexcept {
        return Springs == that.Springs && DamagedGroups == that.DamagedGroups;
    }
};
} //namespace

namespace std {
template<>
struct hash<Line> : hash<std::string> {
    std::size_t operator()(const Line& line) const noexcept {
        return hash<std::string>::operator()(line.Springs);
    }
};
} //namespace std

namespace {
std::unordered_map<Line, std::int64_t> Cache;

auto parse(const std::vector<std::string_view>& input) {
    std::vector<Line> ret;
    for ( auto inputLine : input ) {
        Line       line;
        const auto space = inputLine.find(' ');
        throwIfInvalid(space != std::string_view::npos);
        line.Springs = inputLine.substr(0, space);
        std::ranges::transform(splitString(inputLine.substr(space + 1), ','), std::back_inserter(line.DamagedGroups),
                               convert);
        line.NumberOfUnknown         = std::ranges::count(line.Springs, Unknown);
        line.NumberOfDamagedInList   = std::ranges::count(line.Springs, Damaged);
        line.NumberOfDamagedInGroups = std::ranges::fold_left(line.DamagedGroups, 0, std::plus<>{});
        ret.push_back(std::move(line));
    } //for ( auto inputLine : input )
    return ret;
}

std::int64_t calcNumberOfArrengements(const Line& line) noexcept {
    //Meine Lösung, dauert aber viel zu lange für Part 2
    static_assert(Damaged < Operational);
    std::vector<SpringInfo> inserts(static_cast<std::size_t>(line.NumberOfUnknown), Operational);
    std::ranges::fill_n(inserts.begin(), line.NumberOfDamagedInGroups - line.NumberOfDamagedInList, Damaged);

    std::int64_t number = 0;

    do { //while ( std::ranges::next_permutation(inserts).found )
        ++number;

        auto         insert         = inserts.begin();
        auto         group          = line.DamagedGroups.begin();
        std::int64_t groupRemaining = *group;
        bool         startOfGroup   = true;
        ++group;

        for ( auto spring : line.Springs ) {
            if ( spring == Unknown ) {
                spring = *insert;
                ++insert;
            } //if ( spring == Unknown )

            if ( spring == Damaged ) {
                if ( groupRemaining == 0 ) {
                    --number;
                    break;
                } //if ( groupRemaining == 0 )
                --groupRemaining;
                startOfGroup = false;
            } //if ( spring == Damaged )
            else {
                if ( groupRemaining != 0 && !startOfGroup ) {
                    --number;
                    break;
                } //if ( groupRemaining != 0 && !startOfGroup )

                if ( !startOfGroup ) {
                    groupRemaining = *group;
                    ++group;
                    startOfGroup = true;
                } //if ( !startOfGroup )
            } //else -> if ( spring == Damaged )
        } //for ( auto spring : line.Springs )
    } while ( std::ranges::next_permutation(inserts).found );
    return number;
}

Line unfold(const Line& line) noexcept {
    Line ret;
    ret.Springs = std::format("{0:s}?{0:s}?{0:s}?{0:s}?{0:s}", line.Springs);
    ret.DamagedGroups.resize(line.DamagedGroups.size() * 5);
    auto group = ret.DamagedGroups.begin();
    for ( int i = 0; i < 5; ++i ) {
        group = std::ranges::copy(line.DamagedGroups, group).out;
    } //for ( int i = 0; i < 5; ++i )
    ret.NumberOfDamagedInGroups = line.NumberOfDamagedInGroups * 5;
    ret.NumberOfDamagedInList   = line.NumberOfDamagedInList * 5;
    ret.NumberOfUnknown         = line.NumberOfUnknown * 5 + 4;

    return ret;
}

std::int64_t recurse(Line line) noexcept;

std::int64_t calcRecurse(Line line) noexcept {
    //Von https://pastebin.com/djb8RJ85 geklaut...
    for ( ;; ) {
        if ( line.DamagedGroups.empty() ) {
            //Wir haben nichts mehr zu verteilen. Das passt, wenn es auch keine Damaged mehr gibt.
            return line.Springs.contains(Damaged) ? 0 : 1;
        } //if ( line.DamagedGroups.empty() )

        if ( line.Springs.empty() ) {
            //Keine Springs mehr, aber wür müssten noch kaputte verteilen.
            return 0;
        } //if ( line.Springs.empty() )

        switch ( static_cast<SpringInfo>(line.Springs.front()) ) {
            case Operational : {
                auto pos = line.Springs.find_first_not_of(Operational);
                line.Springs.erase(0, pos);
                break;
            } //case Operational

            case Unknown : {
                line.Springs.front() = Operational;
                auto operational     = recurse(line);
                line.Springs.front() = Damaged;
                auto damaged         = recurse(line);
                return operational + damaged;
            } //case Unknown

            case Damaged : {
                const auto groupLength = static_cast<std::size_t>(line.DamagedGroups.front());
                if ( line.Springs.size() < groupLength ) {
                    //Passt nicht.
                    return 0;
                } //if ( line.Springs.size() < groupLength )

                if ( std::ranges::contains(line.Springs | std::views::take(groupLength), Operational) ) {
                    //Passt auch nicht.
                    return 0;
                } //if ( std::ranges::contains(line.Springs | std::views::take(groupLength), Operational) )

                if ( line.DamagedGroups.size() > 1 ) {
                    if ( line.Springs.size() < groupLength + 1 || line.Springs[groupLength] == Damaged ) {
                        //Gruppe nicht abgeschlossen, oder nicht genug Platz für die nächste Gruppe.
                        return 0;
                    } //if ( line.Springs.size() < groupLength + 1 || line.Springs[groupLength] == Damaged )

                    line.Springs.erase(0, groupLength + 1);
                    line.DamagedGroups.erase(line.DamagedGroups.begin());
                    break;
                } //if ( line.DamagedGroups.size() > 1 )

                line.Springs.erase(0, groupLength);
                line.DamagedGroups.erase(line.DamagedGroups.begin());
                break;
            } //case Damaged
        } //switch ( static_cast<SpringInfo>(line.Springs.front()) )
    } //for ( ;; )
}

std::int64_t recurse(Line line) noexcept {
    const auto iter = Cache.find(line);
    if ( iter != Cache.end() ) {
        return iter->second;
    } //if ( iter != Cache.end() )

    auto result = calcRecurse(line);
    Cache.emplace(std::move(line), result);
    return result;
}
} //namespace

bool challenge12(const std::vector<std::string_view>& input) {
    const auto lines = parse(input);

    std::int64_t sum1 =
        std::ranges::fold_left(lines | std::views::transform(calcNumberOfArrengements), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);
    std::int64_t sum11 = std::ranges::fold_left(lines | std::views::transform(recurse), 0, std::plus<>{});
    myPrint(" == Result of Part 1(.1): {:d} ==\n", sum11);

    std::int64_t sum2 = std::ranges::fold_left(lines | std::views::transform(unfold) | std::views::transform(recurse),
                                               0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 7236 && sum2 == 495;
}

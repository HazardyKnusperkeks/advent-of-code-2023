#include "challenge14.hpp"

#include "print.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <ranges>

namespace {
using MapData = std::vector<std::string>;

struct RowIter {
    public:
    using difference_type   = std::int64_t;
    using value_type        = char;
    using reference         = char&;
    using iterator_category = std::random_access_iterator_tag;

    RowIter(void) noexcept : RowIter{nullptr, 0, 0, 0} {
        return;
    }

    RowIter(MapData* data, std::size_t currentRow, std::size_t rowCount, std::size_t column) noexcept :
            Data{data}, CurrentRow{currentRow}, RowCount{rowCount}, Column{column} {
        return;
    }

    RowIter& operator++(void) noexcept {
        ++CurrentRow;
        return *this;
    }

    RowIter operator++(int) noexcept {
        auto ret = *this;
        operator++();
        return ret;
    }

    RowIter& operator--(void) noexcept {
        --CurrentRow;
        return *this;
    }

    RowIter operator--(int) noexcept {
        auto ret = *this;
        operator--();
        return ret;
    }

    RowIter& operator+=(std::int64_t i) noexcept {
        CurrentRow = static_cast<std::size_t>(static_cast<std::int64_t>(CurrentRow) + i);
        return *this;
    }

    RowIter operator+(std::int64_t i) const noexcept {
        return RowIter{*this} += i;
    }

    RowIter& operator-=(std::int64_t i) noexcept {
        CurrentRow = static_cast<std::size_t>(static_cast<std::int64_t>(CurrentRow) - i);
        return *this;
    }

    RowIter operator-(std::int64_t i) noexcept {
        return RowIter{*this} -= i;
    }

    char& operator*(void) const noexcept {
        return (*Data)[CurrentRow][Column];
    }

    auto operator<=>(const RowIter&) const noexcept = default;

    private:
    MapData*    Data;
    std::size_t CurrentRow;
    std::size_t RowCount;
    std::size_t Column;
};

struct RowRange {
    RowRange(MapData* data, std::size_t rowCount, std::size_t column) noexcept :
            Data{data}, RowCount{rowCount}, Column{column} {
        return;
    }

    auto begin(void) const noexcept {
        return RowIter{Data, 0, RowCount, Column};
    }

    auto end(void) const noexcept {
        return RowIter{Data, RowCount, RowCount, Column};
    }

    bool operator==(const RowRange&) const noexcept {
        return true;
    }

    private:
    MapData*    Data;
    std::size_t RowCount;
    std::size_t Column;
};

struct ColumnSentinel {};

struct ColumnIter {
    public:
    using difference_type = std::int64_t;
    using value_type      = RowRange;

    ColumnIter(MapData* data, std::size_t rowCount, std::size_t column, std::size_t columnCount) noexcept :
            Data{data}, RowCount{rowCount}, Column{column}, ColumnCount{columnCount}, Rows{data, rowCount, column} {
        return;
    }

    ColumnIter& operator++(void) noexcept {
        ++Column;
        Rows = RowRange{Data, RowCount, Column};
        return *this;
    }

    ColumnIter operator++(int) noexcept {
        auto ret = *this;
        operator++();
        return ret;
    }

    const RowRange& operator*(void) const noexcept {
        return Rows;
    }

    bool operator==(const ColumnIter&) const noexcept = default;

    friend bool operator==(const ColumnIter& iter, ColumnSentinel) noexcept {
        return iter.Column == iter.ColumnCount;
    }

    private:
    MapData*    Data;
    std::size_t RowCount;
    std::size_t Column;
    std::size_t ColumnCount;
    RowRange    Rows;
};

struct ColumnRange {
    public:
    ColumnRange(MapData* data, std::size_t rowCount, std::size_t columnCount) noexcept :
            Data{data}, RowCount{rowCount}, ColumnCount{columnCount} {
        return;
    }

    auto begin(void) noexcept {
        return ColumnIter{Data, RowCount, 0, ColumnCount};
    }

    auto end(void) noexcept {
        return ColumnSentinel{};
    }

    private:
    MapData*    Data;
    std::size_t RowCount;
    std::size_t ColumnCount;
};

struct Map {
    MapData     Data;
    std::size_t RowCount;
    std::size_t ColumnCount;

    auto northFirstRange(void) noexcept {
        return ColumnRange{&Data, RowCount, ColumnCount};
    }

    auto southFirstRange(void) noexcept {
        return northFirstRange() |
               std::views::transform([](auto&& range) noexcept { return range | std::views::reverse; });
    }

    auto westFirstRange(void) noexcept {
        return Data | std::views::take(RowCount);
    }

    auto eastFirstRange(void) noexcept {
        return westFirstRange() |
               std::views::transform([](auto&& range) noexcept { return range | std::views::reverse; });
    }
};

auto parse(const std::vector<std::string_view>& input) {
    Map map;
    for ( auto inputLine : input ) {
        map.Data.emplace_back(inputLine);
    } //for ( auto inputLine : input )
    map.RowCount    = map.Data.size();
    map.ColumnCount = map.Data.front().size();
    return map;
}

struct Tilt {
    template<typename R>
    R&& operator()(R&& range) noexcept {
        static std::array lookFor = {'O', '#'};
        for ( auto target = std::ranges::find(range, '.'), end = range.end(); target != end;
              target = std::ranges::find(std::next(target), end, '.') ) {
            if ( auto toMove = std::ranges::find_first_of(std::next(target), end, lookFor.begin(), lookFor.end());
                 toMove != end ) {
                if ( *toMove == 'O' ) {
                    std::swap(*target, *toMove);
                } //if ( *toMove == 'O' )
                else {
                    target = toMove;
                } //else -> if ( *toMove == 'O' )
            } //if ( auto toMove = std::ranges::find(std::next(target), end, lookFor); toMove != end )
        }
        return range;
    }
};

struct CalcLoad {
    template<typename R>
    std::int64_t operator()(R row) noexcept {
        return std::ranges::fold_left(
            std::views::zip_transform([](char field, std::int64_t index) noexcept { return field == 'O' ? index : 0; },
                                      row, std::views::iota(std::int64_t{1})),
            0, std::plus<>{});
    }
};
} //namespace

#include <chrono>

bool challenge14(const std::vector<std::string_view>& input) {
    const auto map    = parse(input);
    auto       runMap = map;

    auto printRange   = [](auto r) {
        for ( char c : r ) {
            myPrint("{}", c);
        }
        myPrint("\n");
    };

    //std::ranges::for_each(runMap.Data, printRange);
    //myPrint("\nEast:\n");
    //std::ranges::for_each(runMap.eastFirstRange(), printRange);
    //myPrint("\nNorth:\n");
    //std::ranges::for_each(runMap.northFirstRange(), printRange);
    //myPrint("\nWest:\n");
    //std::ranges::for_each(runMap.westFirstRange(), printRange);
    //myPrint("\nSouth:\n");
    //std::ranges::for_each(runMap.southFirstRange(), printRange);

    std::ranges::for_each(runMap.northFirstRange(), Tilt{});
    //myPrint("\nFinal:\n");
    //std::ranges::for_each(runMap.Data, printRange);
    std::int64_t sum1 =
        std::ranges::fold_left(runMap.southFirstRange() | std::views::transform(CalcLoad{}), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::map<MapData, std::pair<MapData, int>> cache;
    MapData                                    prev;
    MapData                                    cacheCycleStartData;
    int                                        cacheCycleStart = 0;

    runMap                                                     = map;
    const auto numberOfCycles                                  = 1'000'000'000;

    using Clock                                                = std::chrono::system_clock;
    const auto start                                           = Clock::now();
    for ( auto cycle = 1; cycle <= numberOfCycles; ++cycle ) {
        if ( cycle % (numberOfCycles / 100) == 0 ) {
            myPrint("{:d} Cycles after {}\n", cycle,
                    std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - start));
            myFlush();
        } //if ( cycle % (numberOfCycles / 100) == 0 )

        auto iter = cache.lower_bound(runMap.Data);
        if ( iter != cache.end() && iter->first == runMap.Data ) {
            const auto cycleLength     = cycle - iter->second.second;
            const auto remainingCycles = (numberOfCycles - cycle) % cycleLength;
            myPrint("Cache Hit at Cycle {:d}, matches Cycle {:d} => Length {:d}, Remaining {:d}\n", cycle,
                    iter->second.second, cycleLength, remainingCycles);
            runMap.Data = iter->second.first;
            cycle       = numberOfCycles - remainingCycles;
            continue;
        } //if ( iter != cache.end() && iter->first == runMap.Data )

        prev = runMap.Data;

        std::ranges::for_each(runMap.northFirstRange(), Tilt{});
        std::ranges::for_each(runMap.westFirstRange(), Tilt{});
        std::ranges::for_each(runMap.southFirstRange(), Tilt{});
        std::ranges::for_each(runMap.eastFirstRange(), Tilt{});

        cache.insert(iter, {prev, std::pair{runMap.Data, cycle}});
        //myPrint("Mapping:\n");
        //std::ranges::for_each(prev, printRange);
        //myPrint("\nTo:\n");
        //std::ranges::for_each(runMap.Data, printRange);
        //myPrint("After {:d}\n", cycle);
    } //for ( auto cycle = 1; cycle <= numberOfCycles; ++cycle )

    std::int64_t sum2 =
        std::ranges::fold_left(runMap.southFirstRange() | std::views::transform(CalcLoad{}), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 108935 && sum2 == 32854;
}

/*
Cycle: 0 after 0s
Cycle: 10000000 after 8s
Cycle: 20000000 after 16s
Cycle: 30000000 after 24s
Cycle: 40000000 after 33s
Cycle: 50000000 after 41s
Cycle: 60000000 after 49s
Cycle: 70000000 after 58s
Cycle: 80000000 after 66s
Cycle: 90000000 after 74s
Cycle: 100000000 after 83s
Cycle: 110000000 after 91s
Cycle: 120000000 after 100s
Cycle: 130000000 after 108s
Cycle: 140000000 after 116s
Cycle: 150000000 after 125s
Cycle: 160000000 after 133s
Cycle: 170000000 after 142s
Cycle: 180000000 after 150s
Cycle: 190000000 after 158s
Cycle: 200000000 after 167s
Cycle: 210000000 after 175s
Cycle: 220000000 after 183s
Cycle: 230000000 after 192s
Cycle: 240000000 after 200s
Cycle: 250000000 after 209s
Cycle: 260000000 after 217s
Cycle: 270000000 after 225s
Cycle: 280000000 after 234s
Cycle: 290000000 after 242s
Cycle: 300000000 after 250s
Cycle: 310000000 after 259s
Cycle: 320000000 after 267s
Cycle: 330000000 after 275s
Cycle: 340000000 after 284s
Cycle: 350000000 after 292s
Cycle: 360000000 after 301s
Cycle: 370000000 after 309s
Cycle: 380000000 after 317s
Cycle: 390000000 after 326s
Cycle: 400000000 after 334s
Cycle: 410000000 after 342s
Cycle: 420000000 after 351s
Cycle: 430000000 after 359s
Cycle: 440000000 after 368s
Cycle: 450000000 after 376s
Cycle: 460000000 after 384s
Cycle: 470000000 after 393s
Cycle: 480000000 after 401s
Cycle: 490000000 after 410s
Cycle: 500000000 after 418s
Cycle: 510000000 after 426s
Cycle: 520000000 after 435s
Cycle: 530000000 after 443s
Cycle: 540000000 after 452s
Cycle: 550000000 after 460s
Cycle: 560000000 after 469s
Cycle: 570000000 after 477s
Cycle: 580000000 after 486s
Cycle: 590000000 after 494s
Cycle: 600000000 after 502s
Cycle: 610000000 after 511s
Cycle: 620000000 after 519s
Cycle: 630000000 after 528s
Cycle: 640000000 after 536s
Cycle: 650000000 after 545s
Cycle: 660000000 after 554s
Cycle: 670000000 after 562s
Cycle: 680000000 after 571s
Cycle: 690000000 after 579s
Cycle: 700000000 after 588s
Cycle: 710000000 after 596s
Cycle: 720000000 after 605s
Cycle: 730000000 after 614s
Cycle: 740000000 after 622s
Cycle: 750000000 after 631s
Cycle: 760000000 after 639s
Cycle: 770000000 after 648s
Cycle: 780000000 after 656s
Cycle: 790000000 after 665s
Cycle: 800000000 after 674s
Cycle: 810000000 after 682s
Cycle: 820000000 after 691s
Cycle: 830000000 after 699s
Cycle: 840000000 after 708s
Cycle: 850000000 after 717s
Cycle: 860000000 after 725s
Cycle: 870000000 after 734s
Cycle: 880000000 after 742s
Cycle: 890000000 after 751s
Cycle: 900000000 after 760s
Cycle: 910000000 after 768s
Cycle: 920000000 after 777s
Cycle: 930000000 after 785s
Cycle: 940000000 after 794s
Cycle: 950000000 after 803s
Cycle: 960000000 after 811s
Cycle: 970000000 after 820s
Cycle: 980000000 after 828s
Cycle: 990000000 after 837s
*/

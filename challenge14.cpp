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

    std::ranges::for_each(runMap.northFirstRange(), Tilt{});
    std::int64_t sum1 =
        std::ranges::fold_left(runMap.southFirstRange() | std::views::transform(CalcLoad{}), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::map<MapData, std::pair<MapData, int>> cache;
    MapData                                    prev;
    MapData                                    cacheCycleStartData;

    runMap                                                     = map;
    const auto numberOfCycles                                  = 1'000'000'000;

    using Clock                                                = std::chrono::system_clock;
    const auto start                                           = Clock::now();
    for ( auto cycle = 1; cycle <= numberOfCycles; ++cycle ) {
        auto iter = cache.lower_bound(runMap.Data);
        if ( iter != cache.end() && iter->first == runMap.Data ) {
            const auto cycleLength     = cycle - iter->second.second;
            const auto remainingCycles = (numberOfCycles - cycle) % cycleLength;
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
    } //for ( auto cycle = 1; cycle <= numberOfCycles; ++cycle )

    std::int64_t sum2 =
        std::ranges::fold_left(runMap.southFirstRange() | std::views::transform(CalcLoad{}), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 108935 && sum2 == 100876;
}

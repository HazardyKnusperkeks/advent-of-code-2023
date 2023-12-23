#include "challenge22.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>

namespace {
template<typename T>
struct Coordinate3D {
    T X;
    T Y;
    T Z;

    constexpr bool operator==(const Coordinate3D&) const noexcept  = default;
    constexpr auto operator<=>(const Coordinate3D&) const noexcept = default;
};
} //namespace

namespace std {
template<typename T>
struct hash<Coordinate3D<T>> {
    size_t operator()(const Coordinate3D<T>& c) const noexcept {
        constexpr auto bits = std::numeric_limits<T>::digits / 3;
        constexpr auto mask = ((T{1} << bits) - 1);
        return std::hash<T>{}((c.X << (2 * bits)) | ((c.Y & mask) << bits) | (c.Z & mask));
    }
};
} //namespace std

namespace {
using BrickPos = Coordinate3D<std::uint16_t>;
using BrickId  = std::int16_t;
using XYMap    = std::array<std::array<BrickId, 10>, 10>;
using ZMap     = std::array<XYMap, 350>;

auto posRangeInLevel(void) noexcept {
    auto gen = std::views::iota(0uz, 10uz);
    return std::views::cartesian_product(gen, gen);
}

struct Brick {
    BrickPos Start;
    BrickPos End;

    std::unordered_set<BrickId> Supports;
    std::unordered_set<BrickId> SupportedBy;
    std::unordered_set<BrickId> TransitiveSupporting;

    Brick(BrickPos start, BrickPos end) noexcept : Start{start}, End{end} {
        return;
    }
};

using BrickMap = std::unordered_map<BrickId, Brick>;

auto brickPosRange(const Brick& brick) noexcept {
    return std::views::cartesian_product(std::views::iota(brick.Start.X, static_cast<std::uint16_t>(brick.End.X + 1)),
                                         std::views::iota(brick.Start.Y, static_cast<std::uint16_t>(brick.End.Y + 1)));
}

struct Tower {
    BrickMap                               Bricks;
    ZMap                                   Map;
    std::array<std::int8_t, ZMap{}.size()> BricksPerLevel;

    bool fall(BrickId id) noexcept {
        Brick& brick = Bricks.at(id);
        if ( !brick.SupportedBy.empty() ) {
            //Der ist schon unten.
            return false;
        } //if ( !brick.SupportedBy.empty() )

        bool moved = false;
        while ( true ) {
            if ( brick.Start.Z == 1 ) {
                //Ganz unten.
                return moved;
            } //if ( brick.Start.Z == 1 )

            auto zBelow        = brick.Start.Z - 1u;
            bool hitOtherBrick = false;

            for ( auto [x, y] : brickPosRange(brick) ) {
                if ( auto belowId = Map[zBelow][x][y] ) {
                    //Aufgeschlagen.
                    brick.SupportedBy.insert(belowId);
                    Bricks.at(belowId).Supports.insert(id);
                    hitOtherBrick = true;
                } //if ( auto belowId = Map[z][x][y] )
            } //for ( auto [x, y] : brickPosRange(brick) )

            if ( hitOtherBrick ) {
                return moved;
            } //if ( hitOtherBrick )

            //Wir habens geschafft, also freie Bahn mit Marzipan.
            moved = true;
            for ( auto [x, y] : brickPosRange(brick) ) {
                Map[brick.End.Z][x][y] = 0;
                Map[zBelow][x][y]      = id;
            } //for ( auto [x, y] : brickPosRange(brick) )

            --BricksPerLevel[brick.End.Z];
            ++BricksPerLevel[zBelow];
            --brick.Start.Z;
            --brick.End.Z;
        } //while ( true )
        throwIfInvalid(false);
        return false;
    }

    std::uint16_t nextZLevelWithBrick(std::uint16_t start) const {
        for ( ++start; start < BricksPerLevel.size(); ++start ) {
            if ( BricksPerLevel[start] > 0 ) {
                return start;
            } //if ( BricksPerLevel[start] > 0 )
        } //for ( ++start; start < BricksPerLevel.size(); ++start )
        throwIfInvalid(false);
        return 0;
    }

    auto brickFall(void) {
        std::uint16_t               z              = 0;
        auto                        bricksToHandle = Bricks.size();
        std::unordered_set<BrickId> handledIds;
        int                         bricksFallen = 0;

        while ( bricksToHandle != 0 ) {
            z                                      = nextZLevelWithBrick(z);

            auto                 bricksInThisLevel = BricksPerLevel[z];
            std::vector<BrickId> handledIdsInThisLevel;

            for ( auto [x, y] : posRangeInLevel() ) {
                if ( auto id = Map[z][x][y] ) {
                    if ( std::ranges::contains(handledIdsInThisLevel, id) ) {
                        continue;
                    } //if ( std::ranges::contains(handledIdsInThisLevel, id) )

                    if ( handledIds.contains(id) ) {
                        //Auf einem anderen Level schon behandelt.
                        handledIdsInThisLevel.push_back(id);
                        continue;
                    } //if ( handledIds.contains(id) )

                    if ( fall(id) ) {
                        ++bricksFallen;
                    } //if ( fall(id) )

                    --bricksToHandle;
                    handledIdsInThisLevel.push_back(id);
                    handledIds.insert(id);

                    if ( --bricksInThisLevel == 0 ) {
                        break;
                    } //if ( --bricksInThisLevel == 0 )
                } //if ( auto id = Map[z][x][y] )
            } //for ( auto [x, y] : posRangeInLevel() )
        } //while ( bricksToHandle != 0 )

        return bricksFallen;
    }

    void kill(BrickId id) noexcept {
        Brick brick = Bricks.at(id);
        for ( auto z = brick.Start.Z; z <= brick.End.Z; ++z ) {
            --BricksPerLevel[z];

            for ( auto [x, y] : brickPosRange(brick) ) {
                Map[z][x][y] = 0;
            } //for ( auto [x, y] : brickPosRange(brick) )
        } //for ( auto z = brick.Start.Z; z <= brick.End.Z; ++z )
        Bricks.erase(id);
        for ( auto& [id2, brick2] : Bricks ) {
            brick2.SupportedBy.clear();
        } //for ( auto& [id2, brick2] : Bricks )
        return;
    }

    bool isBrickDisintegrateable(const Brick& brick) const noexcept {
        for ( auto supported : brick.Supports ) {
            const auto& upperBrick = Bricks.at(supported);
            if ( upperBrick.SupportedBy.size() == 1 ) {
                return false;
            } //if ( upperBrick.SupportedBy.size() == 1 )
        } //for ( auto supported : brick.Supports )

        return true;
    }
};

auto convert16(std::string_view in) {
    auto ret = convert(in);
    throwIfInvalid(ret <= std::numeric_limits<std::uint16_t>::max());
    return static_cast<std::uint16_t>(ret);
}

Tower parse(const std::vector<std::string_view>& input) {
    Tower ret{};
    ret.Bricks.reserve(input.size());
    BrickId brickId = 0;
    for ( auto line : input ) {
        ++brickId;
        auto match = ctre::match<R"((\d),(\d),(\d+)~(\d),(\d),(\d+))">(line);
        throwIfInvalid(match);
        Brick brick{{convert16(match.get<1>()), convert16(match.get<2>()), convert16(match.get<3>())},
                    {convert16(match.get<4>()), convert16(match.get<5>()), convert16(match.get<6>())}};

        throwIfInvalid(brick.End.X >= brick.Start.X);
        throwIfInvalid(brick.End.Y >= brick.Start.Y);
        throwIfInvalid(brick.End.Z >= brick.Start.Z);
        ret.Bricks.emplace(brickId, brick);

        for ( std::uint16_t z = brick.Start.Z; z <= brick.End.Z; ++z ) {
            ++ret.BricksPerLevel[z];
            for ( auto [x, y] : brickPosRange(brick) ) {
                ret.Map[z][x][y] = brickId;
            } //for ( auto [x, y] : brickPosRange(brick) )
        } //for ( std::int16_t z = brick.Start.Z, k = 0; k < brick.Height; ++k, ++z )
    } //for ( auto line : input )
    return ret;
}
} //namespace

bool challenge22(const std::vector<std::string_view>& input) {
    auto tower = parse(input);
    tower.brickFall();

    auto isBrickDisintegrateable = [&tower](const Brick& brick) noexcept {
        return tower.isBrickDisintegrateable(brick);
    };

    auto disintegrateable = std::ranges::count_if(tower.Bricks, isBrickDisintegrateable, &BrickMap::value_type::second);
    myPrint(" == Result of Part 1: {:d} ==\n", disintegrateable);

    auto nonSingleDisintegratable =
        tower.Bricks | std::views::filter([isBrickDisintegrateable](const BrickMap::value_type& idAndBrick) noexcept {
            return !isBrickDisintegrateable(idAndBrick.second);
        }) |
        std::views::transform(&BrickMap::value_type::first);

    auto chainDisintegration = [&tower](BrickId id) noexcept {
        Tower newTower{tower};
        newTower.kill(id);
        return newTower.brickFall();
    };

    auto sumDisintegrateable =
        std::ranges::fold_left(nonSingleDisintegratable | std::views::transform(chainDisintegration), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sumDisintegrateable);

    return disintegrateable == 490 && sumDisintegrateable == 96356;
}

#include "challenge24.hpp"

#include "coordinate3d.hpp"
#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <boost/integer/common_factor_rt.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <algorithm>
#include <ranges>

namespace {
struct HailStoneTrajectory {
    Coordinate3D<std::int64_t> Position;
    Coordinate3D<std::int64_t> Velocity;
    int                        Id;

    auto atTime(int t) const noexcept {
        return Position + t * Velocity;
    }
};
} //namespace

namespace std {
template<>
struct formatter<HailStoneTrajectory> : formatter<std::string_view> {
    template<typename C>
    auto format(const HailStoneTrajectory& hst, C& ctx) const {
        return format_to(ctx.out(), "(#{}: X: {} + {}t, Y: {} + {}t, Z: {} + {}t)", hst.Id, hst.Position.X,
                         hst.Velocity.X, hst.Position.Y, hst.Velocity.Y, hst.Position.Z, hst.Velocity.Z);
    }
};

template<typename T>
struct formatter<Coordinate3D<T>> : formatter<std::string_view> {
    template<typename C>
    auto format(const Coordinate3D<T>& c, C& ctx) const {
        return format_to(ctx.out(), "({}, {}, {})", c.X, c.Y, c.Z);
    }
};
} //namespace std

namespace {
std::vector<HailStoneTrajectory> parse(const std::vector<std::string_view>& input) {
    int                              nextId = 0;
    std::vector<HailStoneTrajectory> ret(input.size());
    for ( auto [line, trajectory] : std::views::zip(input, ret) ) {
        const auto match = ctre::match<R"((\d+), (\d+), (\d+) @ (-?\d+), (-?\d+), (-?\d+))">(line);
        throwIfInvalid(match);
        trajectory.Position = {convert(match.get<1>()), convert(match.get<2>()), convert(match.get<3>())};
        trajectory.Velocity = {convert(match.get<4>()), convert(match.get<5>()), convert(match.get<6>())};
        trajectory.Id       = ++nextId;
    } //for ( auto [line, trajectory] : std::views::zip(input, ret) )
    return ret;
}

struct TwoDFunction {
    double M;
    double B;

    double XTimeLimit;
    double XTimeDirection;

    int Id;

    double operator()(double x) const noexcept {
        return M * x + B;
    }

    bool inThePast(double x) const noexcept {
        return (x > XTimeLimit && XTimeDirection < 0) || (x < XTimeLimit && XTimeDirection > 0);
    }
};

TwoDFunction toFunction(const HailStoneTrajectory& trajectory) noexcept {
    TwoDFunction ret;
    const auto   x1    = static_cast<double>(trajectory.Position.X);
    const auto   x2    = x1 + static_cast<double>(trajectory.Velocity.X);
    const auto   y1    = static_cast<double>(trajectory.Position.Y);
    const auto   y2    = y1 + static_cast<double>(trajectory.Velocity.Y);
    ret.M              = (y2 - y1) / (x2 - x1);
    ret.B              = y1 - ret.M * x1;
    ret.XTimeLimit     = static_cast<double>(trajectory.Position.X);
    ret.XTimeDirection = static_cast<double>(trajectory.Velocity.X);
    ret.Id             = trajectory.Id;
    return ret;
}

auto calcCollidingPaths(const std::vector<TwoDFunction>& functions, double lowerLimit, double upperLimit) noexcept {
    auto filter = [&lowerLimit, &upperLimit](const TwoDFunction& function) noexcept {
        const auto low = function(lowerLimit);
        if ( low < lowerLimit && function.M < 0 ) {
            return false;
        } //if ( low < lowerLimit && function.M < 0 )

        if ( low > upperLimit && function.M > 0 ) {
            return false;
        } //if ( low > upperLimit && function.M > 0 )

        const auto high = function(upperLimit);
        if ( low < lowerLimit && high < lowerLimit ) {
            return false;
        } //if ( low < lowerLimit && high < lowerLimit )

        if ( low > upperLimit && high > upperLimit ) {
            return false;
        } //if ( low > upperLimit && high > upperLimit )
        return true;
    };

    auto colliding = [&lowerLimit, &upperLimit](
                         const std::tuple<const TwoDFunction&, const TwoDFunction&> functionsToCheck) noexcept {
        const auto& f1 = std::get<0>(functionsToCheck);
        const auto& f2 = std::get<1>(functionsToCheck);

        if ( f1.Id >= f2.Id ) {
            return false;
        } //if ( f1.Id >= f2.Id )

        if ( f1.M >= f2.M && f1.M <= f2.M ) {
            return f1.B >= f2.B && f1.B <= f2.B;
        } //if ( f1.M >= f2.M && f1.M <= f2.M )

        const auto x = (f2.B - f1.B) / (f1.M - f2.M);
        if ( x < lowerLimit || x > upperLimit ) {
            return false;
        } //if ( x < lowerLimit || x > upperLimit )

        if ( f1.inThePast(x) || f2.inThePast(x) ) {
            return false;
        } //if ( f1.inThePast(x) || f2.inThePast(x) )

        const auto y = f1(x);
        return y >= lowerLimit && y <= upperLimit;
    };

    auto functionsToCheck = functions | std::views::filter(filter);
    auto matrix           = std::views::cartesian_product(functionsToCheck, functionsToCheck);
    return std::ranges::count_if(matrix, colliding);
}

template<typename T>
struct Fraction {
    public:
    T Numerator;
    T Denominator;

    constexpr Fraction(std::int64_t value) noexcept : Fraction{static_cast<T>(value)} {
        return;
    }

    constexpr Fraction(T value) noexcept : Numerator{value}, Denominator{1} {
        return;
    }

    T toBase(void) const {
        Fraction copy{*this};
        copy.normalize();
        throwIfInvalid(copy.Denominator == 1);
        return copy.Numerator;
    }

    bool operator==(const Fraction& that) const noexcept {
        return std::abs(asDouble() - that.asDouble()) <= 1e-15;
    }

    auto operator<=>(const Fraction& that) const noexcept {
        return asDouble() <=> that.asDouble();
    }

    Fraction operator-(Fraction that) const noexcept {
        Fraction ret{*this};
        auto     lcm     = boost::integer::lcm(Denominator, that.Denominator);
        ret.Numerator   *= lcm / ret.Denominator;
        ret.Numerator   -= that.Numerator * (lcm / that.Denominator);
        ret.Denominator  = lcm;
        ret.normalize();
        return ret;
    }

    Fraction operator*(Fraction that) const noexcept {
        Fraction ret{*this};
        ret.Numerator   *= that.Numerator;
        ret.Denominator *= that.Denominator;
        ret.normalize();
        return ret;
    }

    Fraction operator/(Fraction that) const noexcept {
        Fraction ret{*this};
        ret.Numerator   *= that.Denominator;
        ret.Denominator *= that.Numerator;
        ret.normalize();
        return ret;
    }

    private:
    void normalize(void) noexcept {
        auto gcd     = boost::integer::gcd(Numerator, Denominator);
        Numerator   /= gcd;
        Denominator /= gcd;

        if ( Denominator < 0 ) {
            Numerator   = -Numerator;
            Denominator = -Denominator;
        } //if ( Denominator < 0 )
        return;
    }

    double asDouble(void) const noexcept {
        return static_cast<double>(Numerator) / static_cast<double>(Denominator);
    }
};

struct Equation {
    public:
    using F = Fraction<boost::multiprecision::checked_int512_t>;

    F DX;
    F DY;
    F DZ;
    F X;
    F Y;
    F Z;

    F B;

    F& operator[](std::size_t index) noexcept {
        switch ( index ) {
            case 0 : return DX;
            case 1 : return DY;
            case 2 : return DZ;
            case 3 : return X;
            case 4 : return Y;
            case 5 : return Z;
            case 6 : return B;
        } //switch ( index )
        throwIfInvalid(false);
        return B;
    }

    const F& operator[](std::size_t index) const noexcept {
        switch ( index ) {
            case 0 : return DX;
            case 1 : return DY;
            case 2 : return DZ;
            case 3 : return X;
            case 4 : return Y;
            case 5 : return Z;
            case 6 : return B;
        } //switch ( index )
        throwIfInvalid(false);
        return B;
    }

    Equation& operator-=(const Equation& that) noexcept {
        piecewise(that, std::minus<>{});
        return *this;
    }

    Equation operator*(F f) const noexcept {
        Equation ret{*this};
        ret.piecewise(f, std::multiplies<>{});
        return ret;
    }

    Equation& operator/=(F f) noexcept {
        piecewise(f, std::divides<>{});
        return *this;
    }

    private:
    template<typename Fn>
    void piecewise(const Equation& that, Fn fn) noexcept {
        for ( std::size_t i = 0; i < 7; ++i ) {
            operator[](i) = fn(operator[](i), that[i]);
        } //for ( std::size_t i = 0; i < 7; ++i )
        return;
    }

    template<typename Fn>
    void piecewise(F f, Fn fn) noexcept {
        for ( std::size_t i = 0; i < 7; ++i ) {
            operator[](i) = fn(operator[](i), f);
        } //for ( std::size_t i = 0; i < 7; ++i )
        return;
    }
};

struct System {
    public:
    std::array<Equation, 6> Equations;

    System(std::array<Equation, 6> list) noexcept : Equations{list} {
        return;
    }

    bool solve(void) {
        return solveImpl(0);
    }

    Coordinate3D<std::int64_t> getPosition(void) const {
        return {static_cast<std::int64_t>(Equations[3].B.toBase()), static_cast<std::int64_t>(Equations[4].B.toBase()),
                static_cast<std::int64_t>(Equations[5].B.toBase())};
    }

    private:
    bool solveImpl(std::size_t currentRow) {
        if ( currentRow == Equations.size() ) {
            return true;
        } //if ( currentRow == Equations.size() )

        auto& equation = Equations[currentRow];
        auto& field    = equation[currentRow];

        for ( std::size_t offset = 1; field == Equation::F{0}; ++offset ) {
            if ( currentRow == Equations.size() - offset ) {
                return false;
            } //if ( currentRow == Equations.size() - offset )
            std::swap(equation, Equations[currentRow + offset]);
        } //for ( std::size_t offset = 1; field == Equation::F{0}; ++offset )

        equation /= field;

        for ( auto nextRow = currentRow + 1; nextRow < Equations.size(); ++nextRow ) {
            auto& nextEquation  = Equations[nextRow];
            nextEquation       -= equation * nextEquation[currentRow];
        } //for ( auto nextRow = currentRow + 1; nextRow < Equations.size(); ++nextRow )

        if ( !solveImpl(currentRow + 1) ) {
            return false;
        } //if ( !solveImpl(currentRow + 1) )

        for ( std::size_t previousRow = 0; previousRow < currentRow; ++previousRow ) {
            auto& previousEquation  = Equations[previousRow];
            previousEquation       -= equation * previousEquation[currentRow];
        } //for ( std::size_t previousRow = 0; previousRow < currentRow; ++previousRow )

        return true;
    }
};

Coordinate3D<std::int64_t> calcRockThrowPosition(const std::vector<HailStoneTrajectory>& trajectories) {
    // clang-format off
    /* rock(t) = (X, Y, Z) + (DX, DY, DZ) * t
     * hail1(t) = (x, y, z) + (dx, dy, dz) * t
     *
     * Gleichsetzen und nach t auflösen:
     * t = (X - x) / (dx - DX)
     * t = (Y - y) / (dy - DY)
     * t = (Z - z) / (dz - DZ)
     *
     * Wieder gleichsetzen, damit das t verlieren:
     * (X - x) / (dx - DX) = (Y - y) / (dy - DY)
     * (X - x) / (dx - DX) = (Z - z) / (dz - DZ)
     * (Y - y) / (dy - DY) = (Z - z) / (dz - DZ)
     *
     * Beispielhaft nur mit der 1. Gleichung:
     * (X - x) / (dx - DX)               = (Y - y) / (dy - DY)                | * (dx - DX)
     * (X - x)                           = ((Y - y) * (dx - DX)) / (dy - DY)  | * (dy - DY)
     * (X - x) * (dy - DY)               = (Y - y) * (dx - DX)
     * X * dy - x * dy - X * DY + x * DY = Y * dx - y * dx - Y * DX + y * DX  | etwas umsortieren
     * Y * DX - X * DY                   = Y * dx - y * dx + y * DX - X * dy + x * dy
     *
     * Wiederhole mit hail2(t) = (x', y', z') + (dx', dy', dz'):
     * Y * DX - X * DY = Y * dx' - y' * dx' + y' * DX - X * dy' + x' * dy'
     *
     * Kann man gleichsetzen:
     * Y * dx - y * dx + y * DX - X * dy + x * dy = Y * dx' - y' * dx' + y' * DX - X * dy' + x' * dy'
     *
     * Sortiere unbekannte (X, DX, Y, DY) nach links, Rest (bekannt also eine Zahl) nach rechts.
     * (dx - dx') * Y + (y - y') * DX + (dy' - dy) * X + (x' - x) * DY = dx * y - dx' * y' - x * dy + x' * dy'
     *
     * Stelle nun LGS auf: 6 Gleichungen bei 4 unbekannten.
     *                   DX          DY           X             Y        |                       b                       |
     * X/Y 1. und 2: y_1 - y_2   x_2 - x_1   dy_2 - dy_1   dx_1 - dx_2   dx_1 * y_1 - dx_2 * y_2 - dy_1 * x_1 + dy_2 * x_2
     * X/Z 1. und 2: z_1 - z_2   x_2 - x_1   dz_2 - dz_1   dx_1 - dx_2   dx_1 * z_1 - dx_2 * z_2 - dz_1 * x_1 + dz_2 * x_2
     * Y/Z 1. und 2: z_1 - z_2   y_2 - y_1   dz_2 - dz_1   dy_1 - dy_2   dy_1 * z_1 - dy_2 * z_2 - dz_1 * y_1 + dz_2 * y_2
     * X/Y 1. und 3: y_1 - y_3   x_3 - x_1   dy_3 - dy_1   dx_1 - dx_3   dx_1 * y_1 - dx_3 * y_3 - dy_1 * x_1 + dy_3 * x_3
     * X/Z 1. und 3: z_1 - z_3   x_3 - x_1   dz_3 - dz_1   dx_1 - dx_3   dx_1 * z_1 - dx_3 * z_3 - dz_1 * x_1 + dz_3 * x_3
     * Y/Z 1. und 3: z_1 - z_3   y_3 - y_1   dz_3 - dz_1   dy_1 - dy_3   dy_1 * z_1 - dy_3 * z_3 - dz_1 * y_1 + dz_3 * y_3
     */
    // clang-format on

    throwIfInvalid(trajectories.size() >= 3);
    const auto& f1 = trajectories[0];
    const auto& f2 = trajectories[1];
    const auto& f3 = trajectories[2];

    System system{{/*X&Y=*/Equation{/*DX=*/f1.Position.Y - f2.Position.Y, /*DY=*/f2.Position.X - f1.Position.X,
                                    /*DZ=*/0, /*X=*/f2.Velocity.Y - f1.Velocity.Y, /*Y=*/f1.Velocity.X - f2.Velocity.X,
                                    /*Z=*/0, /*B=*/f1.Velocity.X * f1.Position.Y - f2.Velocity.X * f2.Position.Y -
                                                 f1.Velocity.Y * f1.Position.X + f2.Velocity.Y * f2.Position.X},
                   /*X&Z=*/
                   Equation{/*DX=*/f1.Position.Z - f2.Position.Z, /*DY=*/0, /*DZ=*/f2.Position.X - f1.Position.X,
                            /*X=*/f2.Velocity.Z - f1.Velocity.Z, /*Y=*/0, /*Z=*/f1.Velocity.X - f2.Velocity.X,
                            /*B=*/f1.Velocity.X * f1.Position.Z - f2.Velocity.X * f2.Position.Z -
                                f1.Velocity.Z * f1.Position.X + f2.Velocity.Z * f2.Position.X},
                   /*Y&Z=*/
                   Equation{/*DX=*/0, /*DY=*/f1.Position.Z - f2.Position.Z, /*DZ=*/f2.Position.Y - f1.Position.Y,
                            /*X=*/0, /*Y=*/f2.Velocity.Z - f1.Velocity.Z, /*Z=*/f1.Velocity.Y - f2.Velocity.Y,
                            /*B=*/f1.Velocity.Y * f1.Position.Z - f2.Velocity.Y * f2.Position.Z -
                                f1.Velocity.Z * f1.Position.Y + f2.Velocity.Z * f2.Position.Y},
                   /*X&Y=*/
                   Equation{/*DX=*/f1.Position.Y - f3.Position.Y, /*DY=*/f3.Position.X - f1.Position.X, /*DZ=*/0,
                            /*X=*/f3.Velocity.Y - f1.Velocity.Y, /*Y=*/f1.Velocity.X - f3.Velocity.X, /*Z=*/0,
                            /*B=*/f1.Velocity.X * f1.Position.Y - f3.Velocity.X * f3.Position.Y -
                                f1.Velocity.Y * f1.Position.X + f3.Velocity.Y * f3.Position.X},
                   /*X&Z=*/
                   Equation{/*DX=*/f1.Position.Z - f3.Position.Z, /*DY=*/0, /*DZ=*/f3.Position.X - f1.Position.X,
                            /*X=*/f3.Velocity.Z - f1.Velocity.Z, /*Y=*/0, /*Z=*/f1.Velocity.X - f3.Velocity.X,
                            /*B=*/f1.Velocity.X * f1.Position.Z - f3.Velocity.X * f3.Position.Z -
                                f1.Velocity.Z * f1.Position.X + f3.Velocity.Z * f3.Position.X},
                   /*Y&Z=*/
                   Equation{/*DX=*/0, /*DY=*/f1.Position.Z - f3.Position.Z, /*DZ=*/f3.Position.Y - f1.Position.Y,
                            /*X=*/0, /*Y=*/f3.Velocity.Z - f1.Velocity.Z, /*Z=*/f1.Velocity.Y - f3.Velocity.Y,
                            /*B=*/f1.Velocity.Y * f1.Position.Z - f3.Velocity.Y * f3.Position.Z -
                                f1.Velocity.Z * f1.Position.Y + f3.Velocity.Z * f3.Position.Y}}};

    throwIfInvalid(system.solve(), "Solving");

    return system.getPosition();
}
} //namespace

bool challenge24(const std::vector<std::string_view>& input) {
    const auto                trajectories = parse(input);
    std::vector<TwoDFunction> functions(trajectories.size());
    std::ranges::transform(trajectories, functions.begin(), toFunction);

    auto collidingHailStonePaths = calcCollidingPaths(functions, 200'000'000'000'000., 400'000'000'000'000.);
    myPrint(" == Result of Part 1: {:d} ==\n", collidingHailStonePaths);

    auto rockThrowPosition = calcRockThrowPosition(trajectories);
    auto rockThrowSum      = rockThrowPosition.X + rockThrowPosition.Y + rockThrowPosition.Z;
    myPrint(" == Result of Part 2: {:d} ==\n", rockThrowSum);

    return collidingHailStonePaths == 20336 && rockThrowSum == 677'656'046'662'770;
}

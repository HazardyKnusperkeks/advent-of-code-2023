#ifndef COORDINATE3D_HPP
#define COORDINATE3D_HPP

#include <functional>
#include <limits>

template<typename T>
struct Coordinate3D {
    T X;
    T Y;
    T Z;

    constexpr bool operator==(const Coordinate3D&) const noexcept  = default;
    constexpr auto operator<=>(const Coordinate3D&) const noexcept = default;
};

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

#endif //COORDINATE3D_HPP

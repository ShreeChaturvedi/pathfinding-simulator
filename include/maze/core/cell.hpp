#pragma once

#include <cstddef>
#include <functional>

#include "direction.hpp"

/// @brief Grid coordinate for maze navigation.
struct Cell {
    std::size_t row, col;

    /// @brief Return the adjacent cell in the given direction.
    Cell toward(Direction dir) const;
    /// @brief Mutate this cell by moving in the given direction.
    void move(Direction dir);
    /// @brief Check if a direction stays in bounds.
    bool hasDir(Direction dir, std::size_t width, std::size_t height) const;

    /// @brief Equality comparison for coordinates.
    bool operator==(const Cell& other) const;
};

namespace std {
    /// @brief Hash for using Cell in unordered containers.
    template<> struct hash<Cell> {
        size_t operator()(const Cell& c) const noexcept {
            return std::hash<size_t>{}(c.row) ^ (std::hash<size_t>{}(c.col) << 1);
        }
    };
}

/// @brief DirectionMap::operator[] implementation (requires complete Cell type).
inline Direction& DirectionMap::operator[](Cell cell) {
    return map[cell.row][cell.col];
}

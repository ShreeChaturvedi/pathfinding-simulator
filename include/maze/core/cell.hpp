#pragma once

#include <cstddef>
#include <functional>

#include "direction.hpp"

struct Cell {
    std::size_t row, col;

    Cell toward(Direction dir) const;
    void move(Direction dir);
    bool hasDir(Direction dir, std::size_t width, std::size_t height) const;

    bool operator==(const Cell& other) const;
};

namespace std {
    template<> struct hash<Cell> {
        size_t operator()(const Cell& c) const noexcept {
            return std::hash<size_t>{}(c.row) ^ (std::hash<size_t>{}(c.col) << 1);
        }
    };
}

// DirectionMap::operator[] implementation
// which is placed here because it needs complete Cell definition
inline Direction& DirectionMap::operator[](Cell cell) {
    return map[cell.row][cell.col];
}

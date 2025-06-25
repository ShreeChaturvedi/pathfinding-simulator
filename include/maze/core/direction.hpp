#pragma once

#include <cstddef>
#include <string>

struct Cell;  // forward declaration

/// @brief Cardinal movement directions for grid navigation.
enum Direction {
    left, right, up, down, COUNT
};

/// @brief Return the opposite direction.
inline Direction reverse(Direction dir) {
    if (dir == Direction::left) return Direction::right;
    else if (dir == Direction::right) return Direction::left;
    else if (dir == Direction::up) return Direction::down;
    else return Direction::up;
}

/// @brief ASCII glyphs for directional overlays in simple renders.
const char DirectionGlyphs[] = {'-', '-', '|', '|'};

/// @brief 2D map for storing predecessor directions.
struct DirectionMap {
    Direction** map;
    std::size_t width_;
    std::size_t height_;

    /// @brief Allocate a width x height direction map.
    DirectionMap(std::size_t width, std::size_t height)
        : map(new Direction*[height]), width_(width), height_(height) {
        for (std::size_t row = 0; row < height; ++row)
            map[row] = new Direction[width];
    }

    /// @brief Release allocated memory.
    ~DirectionMap() {
        if (map) {
            for (std::size_t row = 0; row < height_; ++row)
                delete[] map[row];
            delete[] map;
        }
    }

    /// @brief Non-copyable to avoid double-free.
    DirectionMap(const DirectionMap&) = delete;
    DirectionMap& operator=(const DirectionMap&) = delete;

    /// @brief Move constructor.
    DirectionMap(DirectionMap&& other) noexcept
        : map(other.map), width_(other.width_), height_(other.height_) {
        other.map = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }

    /// @brief Move assignment.
    DirectionMap& operator=(DirectionMap&& other) noexcept {
        if (this != &other) {
            if (map) {
                for (std::size_t row = 0; row < height_; ++row)
                    delete[] map[row];
                delete[] map;
            }
            map = other.map;
            width_ = other.width_;
            height_ = other.height_;
            other.map = nullptr;
            other.width_ = 0;
            other.height_ = 0;
        }
        return *this;
    }

    /// @brief Access the stored direction for a cell.
    Direction& operator[](Cell cell);  // declare only
};

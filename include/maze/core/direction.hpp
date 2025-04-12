#pragma once

#include <cstddef>
#include <string>

struct Cell;  // forward declaration

enum Direction {
    left, right, up, down, COUNT
};

inline Direction reverse(Direction dir) {
    if (dir == Direction::left) return Direction::right;
    else if (dir == Direction::right) return Direction::left;
    else if (dir == Direction::up) return Direction::down;
    else return Direction::up;
}

const char DirectionGlyphs[] = {'-', '-', '|', '|'};

struct DirectionMap {
    Direction** map;
    std::size_t width_;
    std::size_t height_;

    DirectionMap(std::size_t width, std::size_t height)
        : map(new Direction*[height]), width_(width), height_(height) {
        for (std::size_t row = 0; row < height; ++row)
            map[row] = new Direction[width];
    }

    ~DirectionMap() {
        if (map) {
            for (std::size_t row = 0; row < height_; ++row)
                delete[] map[row];
            delete[] map;
        }
    }

    // Rule of 5: delete copy operations
    DirectionMap(const DirectionMap&) = delete;
    DirectionMap& operator=(const DirectionMap&) = delete;

    // Move constructor
    DirectionMap(DirectionMap&& other) noexcept
        : map(other.map), width_(other.width_), height_(other.height_) {
        other.map = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }

    // Move assignment
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

    Direction& operator[](Cell cell);  // declare only
};

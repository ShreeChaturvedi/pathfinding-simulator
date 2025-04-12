#include "maze/core/cell.hpp"

Cell Cell::toward(Direction dir) const {
    if (dir == Direction::left) return {row, col - 1};
    else if (dir == Direction::right) return {row, col + 1};
    else if (dir == Direction::up) return {row - 1, col};
    else return {row + 1, col};
}

void Cell::move(Direction dir) {
    (*this) = toward(dir);
}

bool Cell::hasDir(Direction dir, std::size_t width, std::size_t height) const {
    if (dir == Direction::left) return col > 0;
    else if (dir == Direction::right) return col < width - 1;
    else if (dir == Direction::up) return row > 0;
    else return row < height - 1;
}

bool Cell::operator==(const Cell& other) const {
    return row == other.row && col == other.col;
}
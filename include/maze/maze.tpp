// maze.tpp - Template implementations for GenericMaze class
// Included at the end of maze.hpp

#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <initializer_list>
#include <numeric>

template <GraphCell G>
GenericMaze<G>::GenericMaze(std::size_t width, std::size_t height)
    : width(width), height(height), grid(new G*[height]) {
        for (std::size_t row = 0; row < height; ++row)
            grid[row] = new G[width];
}

template <GraphCell G>
GenericMaze<G>::~GenericMaze() {
    for (std::size_t row = 0; row < height; ++row) {
        delete[] grid[row];
    }
    delete[] grid;
}

template <GraphCell G>
void GenericMaze<G>::generateRandom(std::vector<G>& cells, float wall_density) {
    if (wall_density < 0.0f || wall_density > 1.0f)
        throw std::invalid_argument("Wall density must be in the range [0, 1]");

    auto is_wall = [](const G& c) { return c.wall; };
    auto first_non_wall = std::partition(cells.begin(), cells.end(), is_wall);

    // Move the walls into their own vector
    std::vector<G> wall_cells(std::make_move_iterator(cells.begin()),
                              std::make_move_iterator(first_non_wall));

    // Erase walls from the original vector
    cells.erase(cells.begin(), first_non_wall);

    static thread_local std::mt19937 gen(std::random_device{}());
    std::bernoulli_distribution dist(wall_density);

    // Helper to select a weighted random cell, with fallback to uniform if all weights are 0
    auto select_weighted = [](const std::vector<G>& pool) -> G {
        static thread_local std::mt19937 rng(std::random_device{}());

        if (pool.empty()) {
            throw std::invalid_argument("Cannot select from empty cell pool");
        }

        float total = std::accumulate(pool.begin(), pool.end(), 0.0f,
            [](float sum, const G& c) { return sum + c.weight; });

        if (total <= 0.0f) {
            // Fallback: uniform random selection when all weights are 0
            std::uniform_int_distribution<std::size_t> idx_dist(0, pool.size() - 1);
            return pool[idx_dist(rng)];
        }

        std::uniform_real_distribution<float> weight_dist(0.0f, total);
        float r = weight_dist(rng);
        for (const auto& c : pool) {
            r -= c.weight;
            if (r <= 0.0f) {
                return c;
            }
        }
        // Fallback in case of floating-point rounding issues
        return pool.back();
    };

    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t col = 0; col < width; ++col) {
            if (dist(gen)) {
                grid[row][col] = select_weighted(wall_cells);
            } else {
                grid[row][col] = select_weighted(cells);
            }
        }
    }
}

template <GraphCell G>
void GenericMaze<G>::generateRandom(std::initializer_list<G> cells, float wall_density) {
    std::vector<G> vec {cells};
    generateRandom(vec, wall_density);
}

template <GraphCell G>
bool GenericMaze<G>::solve(Algorithm algo, Cell start, Cell dest) {
    // Use default destination if sentinel value is passed
    if (start == Cell{0, 0} && dest == Cell{0, 0}) {
        dest = {height - 1, width - 1};
    }

    Path result = [&] {
        switch (algo) {
            case Algorithm::BFS:      return bfs(start, dest);
            case Algorithm::DFS:      return dfs(start, dest);
            case Algorithm::Dijkstra: return dijkstra(start, dest);
        }
    }();

    if (!result.empty()) displayPath(result, start, dest);
    return !result.empty();
}

template <GraphCell G>
std::ostream& operator<<(std::ostream& os, const GenericMaze<G>& maze) {
    for (std::size_t row = 0; row < maze.height; ++row) {
        for (std::size_t col = 0; col < maze.width; ++col) {
            G cell = maze.grid[row][col];
            os << "\033[38;5;" << static_cast<int>(cell.color)
               << "m" << cell.glyph << "\033[0m" << ' ';
        }
        os << '\n';
    }
    return os;
}

template <GraphCell G>
G& GenericMaze<G>::operator[](Cell cell) {
    if (cell.row >= height || cell.col >= width) {
        throw std::out_of_range("Cell (" + std::to_string(cell.row) + ", "
            + std::to_string(cell.col) + ") out of bounds for maze of size "
            + std::to_string(width) + "x" + std::to_string(height));
    }
    return grid[cell.row][cell.col];
}

template <GraphCell G>
const G& GenericMaze<G>::at(Cell cell) const {
    if (cell.row >= height || cell.col >= width) {
        throw std::out_of_range("Cell (" + std::to_string(cell.row) + ", "
            + std::to_string(cell.col) + ") out of bounds for maze of size "
            + std::to_string(width) + "x" + std::to_string(height));
    }
    return grid[cell.row][cell.col];
}

template <GraphCell G>
G& GenericMaze<G>::at_unchecked(Cell cell) {
    return grid[cell.row][cell.col];
}

template <GraphCell G>
const G& GenericMaze<G>::at_unchecked(Cell cell) const {
    return grid[cell.row][cell.col];
}

template <GraphCell G>
void GenericMaze<G>::displayPath(const Path& path, Cell start, Cell dest, const uint16_t step_ms) {
    std::cout << "\033[?1049h\033[?25l\033[H\033[2J";  // Alt screen + hide cursor + clear
    for (const Direction& dir : path) {
        std::cout << "\033[H";
        // Modify cell properties directly (works with any GraphCell type)
        G& cell = at_unchecked(start);
        cell.glyph = DirectionGlyphs[dir];
        cell.color = Color::white;
        cell.wall = false;
        std::cout << *this << std::flush;
        start.move(dir);
        std::this_thread::sleep_for(std::chrono::milliseconds(step_ms));
    }
    std::cout << "\033[?25h\033[?1049l";  // Show cursor + exit alt screen
}

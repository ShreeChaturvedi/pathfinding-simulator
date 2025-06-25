#pragma once

#include <ostream>
#include <cstddef>
#include <concepts>
#include <vector>
#include <initializer_list>
#include <random>
#include <functional>
#include <unordered_set>

#include "core/cell.hpp"
#include "core/graph_cell.hpp"
#include "core/cell_metadata.hpp"
#include "core/direction.hpp"

/// @brief Sequence of directions that forms a path through the maze.
using Path = std::vector<Direction>;
/// @brief Convenience alias for glyph collections.
using Glyphs = std::vector<char>;
/// @brief Callback for visualization during exploration.
using ExploreCallback = std::function<void(
    const Cell&,
    const std::vector<Cell>&,
    const std::unordered_set<Cell>&)>;

/// @brief Pathfinding algorithms supported by the maze.
enum class Algorithm {
    BFS,
    DFS,
    Dijkstra,
    AStar,
    GreedyBestFirst
};

/// @brief Maze generation algorithms supported by the maze.
enum class GenerationAlgorithm {
    RecursiveBacktracker,
    Prim,
    Kruskal
};

/// @brief Maze container storing cells and algorithms.
template <GraphCell G>
class GenericMaze {
public:
    GenericMaze() = delete;
    /// @brief Construct a maze with the given dimensions.
    GenericMaze(std::size_t width, std::size_t height);
    /// @brief Release owned grid memory.
    ~GenericMaze();

    /// @brief Generate a random maze from a weighted cell pool.
    void generateRandom(std::vector<G>& cells, 
        float wall_density = 0.3f);
    /// @brief Generate a random maze from a fixed initializer list.
    void generateRandom(std::initializer_list<G> cells,
        float wall_density = 0.3f);
    /// @brief Generate a perfect maze using a chosen algorithm.
    void generate(GenerationAlgorithm algorithm,
        const G& wall, const G& passage);
    /// @brief Compute a path without rendering.
    Path findPath(Algorithm algorithm,
        Cell start = {0, 0}, Cell dest = {0, 0},
        ExploreCallback on_explore = nullptr);
    /// @brief Compute a path and optionally visualize it.
    bool solve(Algorithm algorithm,
        Cell start = {0, 0}, Cell dest = {0, 0},
        bool visualize = true);

    template <GraphCell T>
    friend std::ostream& operator<<(std::ostream& os,
        const GenericMaze<T>& maze);

    /// @brief Bounds-checked access to a grid cell.
    G& operator[](Cell cell);
    /// @brief Unchecked access to a grid cell.
    G& at_unchecked(Cell cell);
    /// @brief Unchecked access to a grid cell (const).
    const G& at_unchecked(Cell cell) const;

private:
    const std::size_t width, height;
    G** grid;

    /// @brief Bounds-checked access to a grid cell (const).
    const G& at(Cell cell) const;

    Path bfs(Cell start, Cell dest, ExploreCallback on_explore);
    Path dfs(Cell start, Cell dest, ExploreCallback on_explore);
    Path dijkstra(Cell start, Cell dest, ExploreCallback on_explore);
    Path a_star(Cell start, Cell dest, ExploreCallback on_explore);
    Path greedy_best_first(Cell start, Cell dest, ExploreCallback on_explore);
    void fill(const G& cell);
    void generate_recursive_backtracker(const G& wall, const G& passage,
        std::mt19937& rng);
    void generate_prim(const G& wall, const G& passage,
        std::mt19937& rng);
    void generate_kruskal(const G& wall, const G& passage,
        std::mt19937& rng);

    void displayPath(const Path& path, Cell start, Cell dest, 
        const uint16_t step_ms = 100);
};

using Maze = GenericMaze<CellMetaData>;

#include "maze.tpp"
#include "algorithms/pathfinding.tpp"
#include "algorithms/generation.tpp"

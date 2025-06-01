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

using Path = std::vector<Direction>;
using Glyphs = std::vector<char>;
using ExploreCallback = std::function<void(
    const Cell&,
    const std::vector<Cell>&,
    const std::unordered_set<Cell>&)>;

enum class Algorithm {
    BFS,
    DFS,
    Dijkstra,
    AStar,
    GreedyBestFirst
};

enum class GenerationAlgorithm {
    RecursiveBacktracker,
    Prim,
    Kruskal
};

template <GraphCell G>
class GenericMaze {
public:
    GenericMaze() = delete;
    GenericMaze(std::size_t width, std::size_t height);
    ~GenericMaze();

    void generateRandom(std::vector<G>& cells, 
        float wall_density = 0.3f);
    void generateRandom(std::initializer_list<G> cells,
        float wall_density = 0.3f);
    void generate(GenerationAlgorithm algorithm,
        const G& wall, const G& passage);
    Path findPath(Algorithm algorithm,
        Cell start = {0, 0}, Cell dest = {0, 0},
        ExploreCallback on_explore = nullptr);
    bool solve(Algorithm algorithm,
        Cell start = {0, 0}, Cell dest = {0, 0},
        bool visualize = true);

    template <GraphCell T>
    friend std::ostream& operator<<(std::ostream& os,
        const GenericMaze<T>& maze);

    G& operator[](Cell cell);
    G& at_unchecked(Cell cell);
    const G& at_unchecked(Cell cell) const;

private:
    const std::size_t width, height;
    G** grid;

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

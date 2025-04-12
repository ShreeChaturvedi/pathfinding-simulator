#pragma once

#include <ostream>
#include <cstddef>
#include <concepts>
#include <vector>
#include <initializer_list>

#include "core/cell.hpp"
#include "core/graph_cell.hpp"
#include "core/cell_metadata.hpp"
#include "core/direction.hpp"

using Path = std::vector<Direction>;
using Glyphs = std::vector<char>;

enum class Algorithm {
    BFS,
    DFS,
    Dijkstra
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
    bool solve(Algorithm algorithm,
        Cell start = {0, 0}, Cell dest = {0, 0});

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

    Path bfs(Cell start, Cell dest);
    Path dfs(Cell start, Cell dest);
    Path dijkstra(Cell start, Cell dest);

    void displayPath(const Path& path, Cell start, Cell dest, 
        const uint16_t step_ms = 100);
};

using Maze = GenericMaze<CellMetaData>;

#include "maze.tpp"
#include "algorithms/pathfinding.tpp"

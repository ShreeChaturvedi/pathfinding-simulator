#include <catch2/catch_test_macros.hpp>

#include "maze/maze.hpp"

namespace {

// Helper to create a simple open maze
Maze create_open_maze(std::size_t width, std::size_t height) {
    Maze maze(width, height);
    CellMetaData passage{false, ' ', Color::white, 1.0f};
    for (std::size_t r = 0; r < height; ++r) {
        for (std::size_t c = 0; c < width; ++c) {
            maze[{r, c}] = passage;
        }
    }
    return maze;
}

// Helper to block a row completely
void block_row(Maze& maze, std::size_t row, std::size_t width) {
    CellMetaData wall{true, '#', Color::red, 0.0f};
    for (std::size_t c = 0; c < width; ++c) {
        maze[{row, c}] = wall;
    }
}

}  // namespace

TEST_CASE("BFS pathfinding", "[pathfinding][bfs]") {
    SECTION("finds shortest path in open maze") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::BFS, {0, 0}, {4, 4});
        CHECK(found);
    }

    SECTION("returns false when path is blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::BFS, {0, 0}, {4, 4});
        CHECK_FALSE(found);
    }

    SECTION("handles start equals destination") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::BFS, {2, 2}, {2, 2});
        CHECK(found);
    }
}

TEST_CASE("DFS pathfinding", "[pathfinding][dfs]") {
    SECTION("finds a valid path") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::DFS, {0, 0}, {4, 4});
        CHECK(found);
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::DFS, {0, 0}, {4, 4});
        CHECK_FALSE(found);
    }
}

TEST_CASE("Dijkstra pathfinding", "[pathfinding][dijkstra]") {
    SECTION("finds path respecting weights") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::Dijkstra, {0, 0}, {4, 4});
        CHECK(found);
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::Dijkstra, {0, 0}, {4, 4});
        CHECK_FALSE(found);
    }
}

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

float path_cost(Maze& maze, Cell start, const Path& path) {
    float total = 0.0f;
    Cell current = start;
    for (Direction dir : path) {
        current.move(dir);
        total += maze.at_unchecked(current).weight;
    }
    return total;
}

}  // namespace

TEST_CASE("BFS pathfinding", "[pathfinding][bfs]") {
    SECTION("finds shortest path in open maze") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::BFS, {0, 0}, {4, 4}, false);
        CHECK(found);
    }

    SECTION("returns false when path is blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::BFS, {0, 0}, {4, 4}, false);
        CHECK_FALSE(found);
    }

    SECTION("handles start equals destination") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::BFS, {2, 2}, {2, 2}, false);
        CHECK(found);
    }
}

TEST_CASE("DFS pathfinding", "[pathfinding][dfs]") {
    SECTION("finds a valid path") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::DFS, {0, 0}, {4, 4}, false);
        CHECK(found);
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::DFS, {0, 0}, {4, 4}, false);
        CHECK_FALSE(found);
    }
}

TEST_CASE("Dijkstra pathfinding", "[pathfinding][dijkstra]") {
    SECTION("finds path respecting weights") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::Dijkstra, {0, 0}, {4, 4}, false);
        CHECK(found);
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::Dijkstra, {0, 0}, {4, 4}, false);
        CHECK_FALSE(found);
    }

    SECTION("prefers lower cost path over fewer steps") {
        Maze maze(3, 3);
        CellMetaData passage{false, ' ', Color::white, 1.0f};
        for (std::size_t r = 0; r < 3; ++r) {
            for (std::size_t c = 0; c < 3; ++c) {
                maze[{r, c}] = passage;
            }
        }
        maze[{1, 1}].weight = 10.0f;

        Path path = maze.findPath(Algorithm::Dijkstra, {1, 0}, {1, 2});
        REQUIRE_FALSE(path.empty());
        CHECK(path_cost(maze, {1, 0}, path) == Catch::Approx(4.0f));
    }
}

TEST_CASE("A* pathfinding", "[pathfinding][astar]") {
    SECTION("finds path with weighted costs") {
        Maze maze(3, 3);
        CellMetaData passage{false, ' ', Color::white, 1.0f};
        for (std::size_t r = 0; r < 3; ++r) {
            for (std::size_t c = 0; c < 3; ++c) {
                maze[{r, c}] = passage;
            }
        }
        maze[{1, 1}].weight = 10.0f;

        Path path = maze.findPath(Algorithm::AStar, {1, 0}, {1, 2});
        REQUIRE_FALSE(path.empty());
        CHECK(path_cost(maze, {1, 0}, path) == Catch::Approx(4.0f));
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::AStar, {0, 0}, {4, 4}, false);
        CHECK_FALSE(found);
    }
}

TEST_CASE("Greedy Best-First pathfinding", "[pathfinding][greedy]") {
    SECTION("finds a valid path") {
        auto maze = create_open_maze(5, 5);
        bool found = maze.solve(Algorithm::GreedyBestFirst, {0, 0}, {4, 4}, false);
        CHECK(found);
    }

    SECTION("returns false when blocked") {
        auto maze = create_open_maze(5, 5);
        block_row(maze, 2, 5);
        bool found = maze.solve(Algorithm::GreedyBestFirst, {0, 0}, {4, 4}, false);
        CHECK_FALSE(found);
    }
}

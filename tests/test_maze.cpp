#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

#include "maze/maze.hpp"

TEST_CASE("Maze construction", "[maze]") {
    SECTION("creates maze with correct dimensions") {
        Maze maze(10, 15);
        // Access corners to verify dimensions
        CHECK_NOTHROW(maze[{0, 0}]);
        CHECK_NOTHROW(maze[{14, 9}]);
    }
}

TEST_CASE("Maze bounds checking", "[maze]") {
    Maze maze(10, 10);

    SECTION("operator[] throws on out-of-bounds row") {
        CHECK_THROWS_AS(maze[Cell{10, 0}], std::out_of_range);
    }

    SECTION("operator[] throws on out-of-bounds column") {
        CHECK_THROWS_AS(maze[Cell{0, 10}], std::out_of_range);
    }

    SECTION("operator[] throws on both out-of-bounds") {
        CHECK_THROWS_AS(maze[Cell{15, 15}], std::out_of_range);
    }

    SECTION("at_unchecked does not throw") {
        // Just verify it compiles and doesn't crash for valid indices
        CHECK_NOTHROW(maze.at_unchecked({5, 5}));
    }
}

TEST_CASE("Maze random generation", "[maze]") {
    Maze maze(20, 20);

    CellMetaData wall{true, '#', Color::red, 1.0f};
    CellMetaData passage{false, ' ', Color::white, 1.0f};
    std::vector<CellMetaData> cells{wall, passage};

    SECTION("generates without throwing") {
        CHECK_NOTHROW(maze.generateRandom(cells, 0.3f));
    }

    SECTION("throws on invalid wall density") {
        CHECK_THROWS_AS(maze.generateRandom(cells, -0.1f), std::invalid_argument);
        CHECK_THROWS_AS(maze.generateRandom(cells, 1.5f), std::invalid_argument);
    }

    SECTION("handles zero-weight cells") {
        CellMetaData zeroWall{true, '#', Color::red, 0.0f};
        CellMetaData zeroPassage{false, ' ', Color::white, 0.0f};
        std::vector<CellMetaData> zeroCells{zeroWall, zeroPassage};
        CHECK_NOTHROW(maze.generateRandom(zeroCells, 0.5f));
    }
}

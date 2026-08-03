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

TEST_CASE("Maze move semantics", "[maze][move]") {
    CellMetaData marker{false, 'X', Color::green, 2.5f};
    CellMetaData other{false, '.', Color::white, 1.0f};

    SECTION("move construction transfers ownership") {
        Maze original(4, 3);
        original[{1, 2}] = marker;

        Maze moved(std::move(original));
        CHECK(moved[{1, 2}].glyph == 'X');
        CHECK(moved[{1, 2}].weight == 2.5f);
        // Moved-from maze is empty; accessing it would be out of bounds / null.
        // Just ensure the moved maze remains usable for further writes.
        moved[{0, 0}] = other;
        CHECK(moved[{0, 0}].glyph == '.');
    }

    SECTION("move assignment transfers ownership") {
        Maze source(5, 5);
        source[{3, 3}] = marker;

        Maze target(2, 2);
        target[{0, 0}] = other;
        target = std::move(source);

        CHECK(target[{3, 3}].glyph == 'X');
        CHECK(target[{3, 3}].weight == 2.5f);
        target[{4, 4}] = other;
        CHECK(target[{4, 4}].glyph == '.');
    }

    SECTION("self move-assignment is a no-op") {
        Maze maze(3, 3);
        maze[{1, 1}] = marker;
        Maze& ref = maze;
        maze = std::move(ref);
        CHECK(maze[{1, 1}].glyph == 'X');
    }
}

TEST_CASE("Maze bounds checking", "[maze]") {
    Maze maze(10, 10);

    SECTION("operator[] throws on out-of-bounds row") {
        CHECK_THROWS_AS((maze[Cell{10, 0}]), std::out_of_range);
    }

    SECTION("operator[] throws on out-of-bounds column") {
        CHECK_THROWS_AS((maze[Cell{0, 10}]), std::out_of_range);
    }

    SECTION("operator[] throws on both out-of-bounds") {
        CHECK_THROWS_AS((maze[Cell{15, 15}]), std::out_of_range);
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

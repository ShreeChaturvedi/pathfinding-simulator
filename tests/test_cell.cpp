#include <catch2/catch_test_macros.hpp>

#include "maze/core/cell.hpp"

TEST_CASE("Cell navigation", "[cell]") {
    Cell cell{5, 5};

    SECTION("toward returns correct neighbor") {
        CHECK(cell.toward(Direction::left) == Cell{5, 4});
        CHECK(cell.toward(Direction::right) == Cell{5, 6});
        CHECK(cell.toward(Direction::up) == Cell{4, 5});
        CHECK(cell.toward(Direction::down) == Cell{6, 5});
    }

    SECTION("move modifies cell in place") {
        Cell c{3, 3};
        c.move(Direction::right);
        CHECK(c == Cell{3, 4});
    }

    SECTION("hasDir checks boundaries correctly") {
        Cell corner{0, 0};
        CHECK_FALSE(corner.hasDir(Direction::left, 10, 10));
        CHECK_FALSE(corner.hasDir(Direction::up, 10, 10));
        CHECK(corner.hasDir(Direction::right, 10, 10));
        CHECK(corner.hasDir(Direction::down, 10, 10));

        Cell bottomRight{9, 9};
        CHECK(bottomRight.hasDir(Direction::left, 10, 10));
        CHECK(bottomRight.hasDir(Direction::up, 10, 10));
        CHECK_FALSE(bottomRight.hasDir(Direction::right, 10, 10));
        CHECK_FALSE(bottomRight.hasDir(Direction::down, 10, 10));
    }
}

TEST_CASE("Cell equality", "[cell]") {
    CHECK(Cell{1, 2} == Cell{1, 2});
    CHECK_FALSE(Cell{1, 2} == Cell{2, 1});
    CHECK_FALSE(Cell{1, 2} == Cell{1, 3});
}

TEST_CASE("Direction reverse", "[direction]") {
    CHECK(reverse(Direction::left) == Direction::right);
    CHECK(reverse(Direction::right) == Direction::left);
    CHECK(reverse(Direction::up) == Direction::down);
    CHECK(reverse(Direction::down) == Direction::up);
}

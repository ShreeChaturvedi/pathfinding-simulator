#include <catch2/catch_test_macros.hpp>
#include <queue>
#include <vector>

#include "maze/maze.hpp"

namespace {

std::size_t count_passages(const Maze& maze, std::size_t width, std::size_t height) {
    std::size_t total = 0;
    for (std::size_t r = 0; r < height; ++r) {
        for (std::size_t c = 0; c < width; ++c) {
            if (!maze.at_unchecked({r, c}).wall) {
                ++total;
            }
        }
    }
    return total;
}

std::size_t count_reachable_passages(const Maze& maze,
    std::size_t width, std::size_t height) {
    Cell start{0, 0};
    bool found = false;
    for (std::size_t r = 0; r < height && !found; ++r) {
        for (std::size_t c = 0; c < width; ++c) {
            if (!maze.at_unchecked({r, c}).wall) {
                start = {r, c};
                found = true;
                break;
            }
        }
    }

    if (!found) return 0;

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::queue<Cell> queue;
    visited[start.row][start.col] = true;
    queue.push(start);

    std::size_t count = 0;
    while (!queue.empty()) {
        Cell current = queue.front();
        queue.pop();
        ++count;

        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};

        for (int i = 0; i < 4; ++i) {
            int nr = static_cast<int>(current.row) + dr[i];
            int nc = static_cast<int>(current.col) + dc[i];
            if (nr < 0 || nc < 0) continue;
            if (nr >= static_cast<int>(height) || nc >= static_cast<int>(width)) continue;

            Cell next{static_cast<std::size_t>(nr), static_cast<std::size_t>(nc)};
            if (visited[next.row][next.col]) continue;
            if (maze.at_unchecked(next).wall) continue;
            visited[next.row][next.col] = true;
            queue.push(next);
        }
    }

    return count;
}

void check_connected_generation(GenerationAlgorithm algorithm) {
    constexpr std::size_t width = 21;
    constexpr std::size_t height = 21;
    Maze maze(width, height);
    CellMetaData wall{true, '#', Color::red, 1.0f};
    CellMetaData passage{false, ' ', Color::white, 1.0f};

    maze.generate(algorithm, wall, passage);

    std::size_t total = count_passages(maze, width, height);
    std::size_t reachable = count_reachable_passages(maze, width, height);

    CHECK(total > 0);
    CHECK(reachable == total);
}

}  // namespace

TEST_CASE("Maze generation algorithms produce connected passages", "[generation]") {
    SECTION("Recursive Backtracker") {
        check_connected_generation(GenerationAlgorithm::RecursiveBacktracker);
    }

    SECTION("Prim") {
        check_connected_generation(GenerationAlgorithm::Prim);
    }

    SECTION("Kruskal") {
        check_connected_generation(GenerationAlgorithm::Kruskal);
    }
}

// pathfinding.tpp - Template implementations for pathfinding algorithms
// Included at the end of maze.hpp

#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <algorithm>

template <GraphCell G>
Path GenericMaze<G>::bfs(Cell start, Cell dest) {
    DirectionMap dir_map(width, height);
    std::unordered_set<Cell> visited;
    std::queue<Cell> queue;

    visited.insert(start);
    queue.push(start);

    int depth = 0;

    while (!queue.empty()) {
        ++depth;
        std::size_t size = queue.size();
        while (size-- > 0) {
            Cell cell = queue.front();
            queue.pop();
            if (cell == dest) {
                Path result(depth - 1);
                int i = depth - 2;
                while (i >= 0) {
                    result[i] = dir_map[cell];
                    cell.move(reverse(dir_map[cell]));
                    --i;
                }
                return result;
            }

            for (std::uint8_t d = 0; d < Direction::COUNT; ++d) {
                Direction dir = static_cast<Direction>(d);
                if (cell.hasDir(dir, width, height)) {
                    Cell new_cell = cell.toward(dir);
                    if (at(new_cell).wall || visited.contains(new_cell)) continue;
                    queue.push(new_cell);
                    visited.insert(new_cell);
                    dir_map[new_cell] = dir;
                }
            }
        }
    }
    return {};
}

template <GraphCell G>
Path GenericMaze<G>::dfs(Cell start, Cell dest) {
    if (start == dest) return {};

    DirectionMap dir_map(width, height);
    std::unordered_set<Cell> visited;
    std::stack<Cell> stack;

    visited.insert(start);
    stack.push(start);

    while (!stack.empty()) {
        Cell cell = stack.top();
        stack.pop();

        for (std::uint8_t d = 0; d < Direction::COUNT; ++d) {
            Direction dir = static_cast<Direction>(d);
            if (cell.hasDir(dir, width, height)) {
                Cell neighbor = cell.toward(dir);
                if (at(neighbor).wall || visited.contains(neighbor)) continue;

                visited.insert(neighbor);
                dir_map[neighbor] = dir;

                if (neighbor == dest) {
                    // Reconstruct path
                    Path result;
                    Cell current = dest;
                    while (!(current == start)) {
                        result.push_back(dir_map[current]);
                        current.move(reverse(dir_map[current]));
                    }
                    std::reverse(result.begin(), result.end());
                    return result;
                }

                stack.push(neighbor);
            }
        }
    }
    return {};
}

template <GraphCell G>
Path GenericMaze<G>::dijkstra(Cell start, Cell dest) {
    // TODO: Implement Dijkstra pathfinding
    return {};
}

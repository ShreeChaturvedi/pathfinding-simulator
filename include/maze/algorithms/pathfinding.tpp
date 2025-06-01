// pathfinding.tpp - Template implementations for pathfinding algorithms
// Included at the end of maze.hpp

#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <cmath>

inline float manhattan_distance(const Cell& a, const Cell& b) {
    std::size_t dr = (a.row > b.row) ? (a.row - b.row) : (b.row - a.row);
    std::size_t dc = (a.col > b.col) ? (a.col - b.col) : (b.col - a.col);
    return static_cast<float>(dr + dc);
}

inline float euclidean_distance(const Cell& a, const Cell& b) {
    float dr = static_cast<float>(a.row > b.row ? (a.row - b.row) : (b.row - a.row));
    float dc = static_cast<float>(a.col > b.col ? (a.col - b.col) : (b.col - a.col));
    return std::sqrt(dr * dr + dc * dc);
}

template <GraphCell G>
Path GenericMaze<G>::bfs(Cell start, Cell dest, ExploreCallback on_explore) {
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
            if (on_explore) {
                std::vector<Cell> frontier;
                frontier.reserve(queue.size());
                auto temp = queue;
                while (!temp.empty()) {
                    frontier.push_back(temp.front());
                    temp.pop();
                }
                on_explore(cell, frontier, visited);
            }
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
Path GenericMaze<G>::dfs(Cell start, Cell dest, ExploreCallback on_explore) {
    if (start == dest) return {};

    DirectionMap dir_map(width, height);
    std::unordered_set<Cell> visited;
    std::stack<Cell> stack;

    visited.insert(start);
    stack.push(start);

    while (!stack.empty()) {
        Cell cell = stack.top();
        stack.pop();
        if (on_explore) {
            std::vector<Cell> frontier;
            frontier.reserve(stack.size());
            auto temp = stack;
            while (!temp.empty()) {
                frontier.push_back(temp.top());
                temp.pop();
            }
            on_explore(cell, frontier, visited);
        }

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
Path GenericMaze<G>::dijkstra(Cell start, Cell dest, ExploreCallback on_explore) {
    if (start == dest) return {};

    DirectionMap dir_map(width, height);
    std::unordered_map<Cell, float> dist;
    std::unordered_set<Cell> visited;

    // Min-heap: (distance, cell)
    using PQEntry = std::pair<float, Cell>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;

    dist[start] = 0.0f;
    pq.emplace(0.0f, start);

    while (!pq.empty()) {
        auto [d, cell] = pq.top();
        pq.pop();

        // Skip stale entries
        if (dist.contains(cell) && d > dist[cell]) continue;
        visited.insert(cell);

        if (on_explore) {
            std::vector<Cell> frontier;
            frontier.reserve(pq.size());
            auto temp = pq;
            while (!temp.empty()) {
                frontier.push_back(temp.top().second);
                temp.pop();
            }
            on_explore(cell, frontier, visited);
        }

        if (cell == dest) {
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

        for (std::uint8_t di = 0; di < Direction::COUNT; ++di) {
            Direction dir = static_cast<Direction>(di);
            if (cell.hasDir(dir, width, height)) {
                Cell neighbor = cell.toward(dir);
                const G& neighbor_data = at(neighbor);
                if (neighbor_data.wall) continue;

                float new_dist = dist[cell] + neighbor_data.weight;
                if (!dist.contains(neighbor) || new_dist < dist[neighbor]) {
                    dist[neighbor] = new_dist;
                    dir_map[neighbor] = dir;
                    pq.emplace(new_dist, neighbor);
                }
            }
        }
    }
    return {};
}

template <GraphCell G>
Path GenericMaze<G>::a_star(Cell start, Cell dest, ExploreCallback on_explore) {
    if (start == dest) return {};

    DirectionMap dir_map(width, height);
    std::unordered_map<Cell, float> g_score;
    std::unordered_set<Cell> visited;

    using PQEntry = std::pair<float, Cell>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;

    g_score[start] = 0.0f;
    pq.emplace(manhattan_distance(start, dest), start);

    while (!pq.empty()) {
        auto [f_score, cell] = pq.top();
        pq.pop();

        if (cell == dest) {
            Path result;
            Cell current = dest;
            while (!(current == start)) {
                result.push_back(dir_map[current]);
                current.move(reverse(dir_map[current]));
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        if (g_score.contains(cell)) {
            float expected_f = g_score[cell] + manhattan_distance(cell, dest);
            if (f_score > expected_f) continue;
        }
        visited.insert(cell);

        if (on_explore) {
            std::vector<Cell> frontier;
            frontier.reserve(pq.size());
            auto temp = pq;
            while (!temp.empty()) {
                frontier.push_back(temp.top().second);
                temp.pop();
            }
            on_explore(cell, frontier, visited);
        }

        for (std::uint8_t di = 0; di < Direction::COUNT; ++di) {
            Direction dir = static_cast<Direction>(di);
            if (!cell.hasDir(dir, width, height)) continue;

            Cell neighbor = cell.toward(dir);
            const G& neighbor_data = at(neighbor);
            if (neighbor_data.wall) continue;

            float tentative_g = g_score[cell] + neighbor_data.weight;
            if (!g_score.contains(neighbor) || tentative_g < g_score[neighbor]) {
                g_score[neighbor] = tentative_g;
                dir_map[neighbor] = dir;
                float f = tentative_g + manhattan_distance(neighbor, dest);
                pq.emplace(f, neighbor);
            }
        }
    }
    return {};
}

template <GraphCell G>
Path GenericMaze<G>::greedy_best_first(Cell start, Cell dest, ExploreCallback on_explore) {
    if (start == dest) return {};

    DirectionMap dir_map(width, height);
    std::unordered_set<Cell> visited;

    using PQEntry = std::pair<float, Cell>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;

    visited.insert(start);
    pq.emplace(manhattan_distance(start, dest), start);

    while (!pq.empty()) {
        auto [priority, cell] = pq.top();
        pq.pop();

        if (on_explore) {
            std::vector<Cell> frontier;
            frontier.reserve(pq.size());
            auto temp = pq;
            while (!temp.empty()) {
                frontier.push_back(temp.top().second);
                temp.pop();
            }
            on_explore(cell, frontier, visited);
        }

        if (cell == dest) {
            Path result;
            Cell current = dest;
            while (!(current == start)) {
                result.push_back(dir_map[current]);
                current.move(reverse(dir_map[current]));
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        for (std::uint8_t di = 0; di < Direction::COUNT; ++di) {
            Direction dir = static_cast<Direction>(di);
            if (!cell.hasDir(dir, width, height)) continue;

            Cell neighbor = cell.toward(dir);
            if (at(neighbor).wall || visited.contains(neighbor)) continue;

            visited.insert(neighbor);
            dir_map[neighbor] = dir;
            pq.emplace(manhattan_distance(neighbor, dest), neighbor);
        }
    }
    return {};
}

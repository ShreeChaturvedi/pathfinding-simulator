// generation.tpp - Template implementations for maze generation algorithms
// Included at the end of maze.hpp

#include <vector>
#include <random>
#include <algorithm>

template <GraphCell G>
void GenericMaze<G>::fill(const G& cell) {
    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t col = 0; col < width; ++col) {
            grid[row][col] = cell;
        }
    }
}

template <GraphCell G>
void GenericMaze<G>::generate(GenerationAlgorithm algorithm, const G& wall, const G& passage) {
    std::mt19937 rng(std::random_device{}());
    switch (algorithm) {
        case GenerationAlgorithm::RecursiveBacktracker:
            generate_recursive_backtracker(wall, passage, rng);
            break;
        case GenerationAlgorithm::Prim:
            generate_prim(wall, passage, rng);
            break;
        case GenerationAlgorithm::Kruskal:
            generate_kruskal(wall, passage, rng);
            break;
    }
}

template <GraphCell G>
void GenericMaze<G>::generate_recursive_backtracker(const G& wall, const G& passage,
    std::mt19937& rng) {
    fill(wall);

    const std::size_t node_rows = (height - 1) / 2;
    const std::size_t node_cols = (width - 1) / 2;
    if (node_rows == 0 || node_cols == 0) {
        fill(passage);
        return;
    }

    auto node_index = [node_cols](Cell cell) {
        return (cell.row / 2) * node_cols + (cell.col / 2);
    };

    std::vector<bool> visited(node_rows * node_cols, false);
    std::uniform_int_distribution<std::size_t> row_dist(0, node_rows - 1);
    std::uniform_int_distribution<std::size_t> col_dist(0, node_cols - 1);

    Cell start{row_dist(rng) * 2 + 1, col_dist(rng) * 2 + 1};
    visited[node_index(start)] = true;
    at_unchecked(start) = passage;

    std::vector<Cell> stack;
    stack.push_back(start);

    const int dr[4] = {-2, 2, 0, 0};
    const int dc[4] = {0, 0, -2, 2};

    while (!stack.empty()) {
        Cell current = stack.back();
        std::vector<Cell> neighbors;

        for (int i = 0; i < 4; ++i) {
            int nr = static_cast<int>(current.row) + dr[i];
            int nc = static_cast<int>(current.col) + dc[i];
            if (nr <= 0 || nc <= 0) continue;
            if (nr >= static_cast<int>(height - 1) || nc >= static_cast<int>(width - 1)) continue;
            Cell neighbor{static_cast<std::size_t>(nr), static_cast<std::size_t>(nc)};
            if (!visited[node_index(neighbor)]) {
                neighbors.push_back(neighbor);
            }
        }

        if (neighbors.empty()) {
            stack.pop_back();
            continue;
        }

        std::uniform_int_distribution<std::size_t> pick(0, neighbors.size() - 1);
        Cell neighbor = neighbors[pick(rng)];

        Cell between{
            (current.row + neighbor.row) / 2,
            (current.col + neighbor.col) / 2
        };
        at_unchecked(between) = passage;
        at_unchecked(neighbor) = passage;
        visited[node_index(neighbor)] = true;
        stack.push_back(neighbor);
    }
}

template <GraphCell G>
void GenericMaze<G>::generate_prim(const G& wall, const G& passage,
    std::mt19937& rng) {
    fill(wall);

    const std::size_t node_rows = (height - 1) / 2;
    const std::size_t node_cols = (width - 1) / 2;
    if (node_rows == 0 || node_cols == 0) {
        fill(passage);
        return;
    }

    auto node_index = [node_cols](Cell cell) {
        return (cell.row / 2) * node_cols + (cell.col / 2);
    };

    struct FrontierEdge {
        Cell from;
        Cell to;
    };

    std::vector<bool> visited(node_rows * node_cols, false);
    std::uniform_int_distribution<std::size_t> row_dist(0, node_rows - 1);
    std::uniform_int_distribution<std::size_t> col_dist(0, node_cols - 1);

    Cell start{row_dist(rng) * 2 + 1, col_dist(rng) * 2 + 1};
    visited[node_index(start)] = true;
    at_unchecked(start) = passage;

    std::vector<FrontierEdge> frontier;
    auto add_frontier = [&](Cell cell) {
        const int dr[4] = {-2, 2, 0, 0};
        const int dc[4] = {0, 0, -2, 2};
        for (int i = 0; i < 4; ++i) {
            int nr = static_cast<int>(cell.row) + dr[i];
            int nc = static_cast<int>(cell.col) + dc[i];
            if (nr <= 0 || nc <= 0) continue;
            if (nr >= static_cast<int>(height - 1) || nc >= static_cast<int>(width - 1)) continue;
            Cell neighbor{static_cast<std::size_t>(nr), static_cast<std::size_t>(nc)};
            if (!visited[node_index(neighbor)]) {
                frontier.push_back({cell, neighbor});
            }
        }
    };

    add_frontier(start);

    while (!frontier.empty()) {
        std::uniform_int_distribution<std::size_t> pick(0, frontier.size() - 1);
        std::size_t idx = pick(rng);
        FrontierEdge edge = frontier[idx];
        frontier[idx] = frontier.back();
        frontier.pop_back();

        if (visited[node_index(edge.to)]) {
            continue;
        }

        Cell between{
            (edge.from.row + edge.to.row) / 2,
            (edge.from.col + edge.to.col) / 2
        };
        at_unchecked(between) = passage;
        at_unchecked(edge.to) = passage;
        visited[node_index(edge.to)] = true;
        add_frontier(edge.to);
    }
}

template <GraphCell G>
void GenericMaze<G>::generate_kruskal(const G& wall, const G& passage,
    std::mt19937& rng) {
    fill(wall);

    const std::size_t node_rows = (height - 1) / 2;
    const std::size_t node_cols = (width - 1) / 2;
    if (node_rows == 0 || node_cols == 0) {
        fill(passage);
        return;
    }

    auto node_index = [node_cols](Cell cell) {
        return (cell.row / 2) * node_cols + (cell.col / 2);
    };

    struct Edge {
        Cell a;
        Cell b;
    };

    std::vector<Edge> edges;
    edges.reserve(node_rows * node_cols * 2);

    for (std::size_t r = 0; r < node_rows; ++r) {
        for (std::size_t c = 0; c < node_cols; ++c) {
            Cell node{2 * r + 1, 2 * c + 1};
            at_unchecked(node) = passage;
            if (r + 1 < node_rows) {
                edges.push_back({node, Cell{2 * (r + 1) + 1, 2 * c + 1}});
            }
            if (c + 1 < node_cols) {
                edges.push_back({node, Cell{2 * r + 1, 2 * (c + 1) + 1}});
            }
        }
    }

    std::shuffle(edges.begin(), edges.end(), rng);

    struct DisjointSet {
        std::vector<std::size_t> parent;
        std::vector<std::size_t> rank;

        explicit DisjointSet(std::size_t size)
            : parent(size), rank(size, 0) {
            for (std::size_t i = 0; i < size; ++i) parent[i] = i;
        }

        std::size_t find(std::size_t x) {
            if (parent[x] != x) parent[x] = find(parent[x]);
            return parent[x];
        }

        bool unite(std::size_t a, std::size_t b) {
            std::size_t ra = find(a);
            std::size_t rb = find(b);
            if (ra == rb) return false;
            if (rank[ra] < rank[rb]) {
                parent[ra] = rb;
            } else if (rank[ra] > rank[rb]) {
                parent[rb] = ra;
            } else {
                parent[rb] = ra;
                ++rank[ra];
            }
            return true;
        }
    };

    DisjointSet dsu(node_rows * node_cols);

    for (const auto& edge : edges) {
        std::size_t a = node_index(edge.a);
        std::size_t b = node_index(edge.b);
        if (dsu.unite(a, b)) {
            Cell between{
                (edge.a.row + edge.b.row) / 2,
                (edge.a.col + edge.b.col) / 2
            };
            at_unchecked(between) = passage;
        }
    }
}

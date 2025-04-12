// #include <print>

#include "maze/maze.hpp"

int main() {
    Maze maze(80, 80);

    CellMetaData wall_cell {
        .wall = true,
        .glyph = '#',
        .color = Color::red,
        .weight = 5.0f
    };
    
    CellMetaData fancy_wall_cell {
        .wall = true,
        .glyph = '@',
        .color = Color::magenta,
        .weight = 1.0f
    };

    CellMetaData road_cell {
        .wall = false,
        .glyph = ' ',
        .color = Color::gray,
        .weight = 20.0f
    };

    CellMetaData tree_cell {
        .wall = false,
        .glyph = 'T',
        .color = Color::green,
        .weight = 3.0f
    };

    CellMetaData water_cell {
        .wall = true,
        .glyph = '~',
        .color = Color::blue,
        .weight = 16.0f
    };

    std::vector<CellMetaData> cells {
        wall_cell, fancy_wall_cell, road_cell, tree_cell, water_cell
    };

    maze.generateRandom(cells);
    maze.solve(Algorithm::BFS, {1, 1}, {79, 79});

    return 0;
}

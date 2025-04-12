#pragma once

enum class Color {
    // ansii 256 colors
    black = 0,
    red = 1,
    green = 2,
    blue = 4,
    yellow = 3,
    cyan = 6,
    magenta = 13,
    white = 15,
    gray = 7
};

struct CellMetaData {
    bool wall;
    char glyph;
    Color color;
    float weight;
};

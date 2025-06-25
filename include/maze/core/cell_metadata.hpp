#pragma once

/// @brief ANSI 256-color palette indices used for rendering.
enum class Color {
    // ANSI 256 colors
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

/// @brief Render and traversal metadata for each maze cell.
struct CellMetaData {
    /// @brief True if the cell is not passable.
    bool wall;
    /// @brief Glyph used for rendering.
    char glyph;
    /// @brief Color used for rendering.
    Color color;
    /// @brief Traversal cost (used by weighted algorithms).
    float weight;
};

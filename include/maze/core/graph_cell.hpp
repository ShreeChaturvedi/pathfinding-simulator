#pragma once

#include <concepts>
#include "cell_metadata.hpp"

template <typename T>
concept GraphCell = requires(T t) {
    {t.wall} -> std::convertible_to<bool>;
    {t.glyph} -> std::convertible_to<char>;
    {t.color} -> std::convertible_to<Color>;
    {t.weight} -> std::convertible_to<float>;
};

// SFINAE

#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "maze/maze.hpp"

namespace maze::ui {

/// @brief FTXUI-based interactive maze visualizer.
class MazeApp {
public:
    /// @brief Construct the app with the given maze dimensions.
    MazeApp(std::size_t width, std::size_t height);
    /// @brief Ensure background worker threads are stopped.
    ~MazeApp();

    /// @brief Run the interactive UI loop.
    void run();

private:
    struct TerrainPreset {
        std::string name;
        std::vector<CellMetaData> passages;
    };

    void regenerate();
    void begin_solve();
    void stop_solver();
    void rebuild_solution_index();

    bool handle_event(const ftxui::Event& event);
    bool handle_grid_event(const ftxui::Event& event);
    bool handle_menu_event(const ftxui::Event& event);

    ftxui::Element render_grid();
    ftxui::Element render_sidebar();

    void apply_terrain();
    Cell find_first_passage(bool from_end) const;
    std::vector<Cell> build_cell_path(const Path& path) const;

    static ftxui::Color map_color(::Color color);

    const std::size_t width_;
    const std::size_t height_;

    Maze maze_;
    std::mt19937 rng_;

    const CellMetaData wall_cell_;
    const CellMetaData passage_cell_;
    std::vector<TerrainPreset> terrains_;

    std::vector<std::string> algorithm_labels_;
    std::vector<Algorithm> algorithm_values_;
    std::vector<std::string> generator_labels_;
    std::vector<GenerationAlgorithm> generator_values_;
    std::vector<std::string> terrain_labels_;

    int algorithm_index_ = 0;
    int generator_index_ = 0;
    int terrain_index_ = 0;
    int last_terrain_index_ = 0;

    std::atomic<bool> solving_{false};
    std::atomic<bool> stop_requested_{false};
    std::mutex state_mutex_;
    std::thread solver_thread_;

    Cell start_{1, 1};
    Cell dest_{1, 1};
    Cell cursor_{1, 1};
    std::optional<Cell> current_cell_;
    std::unordered_set<Cell> visited_;
    std::unordered_set<Cell> frontier_;
    std::unordered_map<Cell, std::size_t> solution_index_;
    std::vector<Cell> solution_cells_;
    std::size_t pulse_index_ = 0;
    bool show_solution_ = false;
    bool focus_on_grid_ = true;

    ftxui::Component algorithm_menu_;
    ftxui::Component generator_menu_;
    ftxui::Component terrain_menu_;
    ftxui::Component menu_container_;
    ftxui::Component grid_renderer_;
    ftxui::Component root_;
    ftxui::Component renderer_;

    ftxui::ScreenInteractive* screen_ = nullptr;
};

}  // namespace maze::ui

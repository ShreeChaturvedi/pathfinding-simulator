#include "maze/ui/app.hpp"

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <chrono>

namespace maze::ui {

namespace {

constexpr std::chrono::milliseconds kExploreDelay{12};
constexpr std::chrono::milliseconds kPulseDelay{24};

}  // namespace

MazeApp::MazeApp(std::size_t width, std::size_t height)
    : width_(width),
      height_(height),
      maze_(width, height),
      rng_(std::random_device{}()),
      wall_cell_{true, '#', Color::gray, 10.0f},
      passage_cell_{false, ' ', Color::white, 1.0f} {
    terrains_ = {
        {"Classic", {passage_cell_}},
        {"Forest", {
            {false, '.', Color::green, 1.0f},
            {false, ',', Color::green, 2.0f},
            {false, '~', Color::cyan, 4.0f}
        }},
        {"Ruins", {
            {false, '.', Color::gray, 1.0f},
            {false, ':', Color::yellow, 3.0f},
            {false, '*', Color::red, 6.0f}
        }}
    };

    algorithm_labels_ = {"BFS", "DFS", "Dijkstra", "A*", "Greedy Best-First"};
    algorithm_values_ = {
        Algorithm::BFS,
        Algorithm::DFS,
        Algorithm::Dijkstra,
        Algorithm::AStar,
        Algorithm::GreedyBestFirst
    };

    generator_labels_ = {"Recursive Backtracker", "Prim", "Kruskal"};
    generator_values_ = {
        GenerationAlgorithm::RecursiveBacktracker,
        GenerationAlgorithm::Prim,
        GenerationAlgorithm::Kruskal
    };

    terrain_labels_.reserve(terrains_.size());
    for (const auto& preset : terrains_) terrain_labels_.push_back(preset.name);

    algorithm_menu_ = ftxui::Menu(&algorithm_labels_, &algorithm_index_);
    generator_menu_ = ftxui::Menu(&generator_labels_, &generator_index_);
    terrain_menu_ = ftxui::Menu(&terrain_labels_, &terrain_index_);
    menu_container_ = ftxui::Container::Vertical({
        algorithm_menu_, generator_menu_, terrain_menu_
    });

    grid_renderer_ = ftxui::Renderer([this] { return render_grid(); });
    root_ = ftxui::Container::Horizontal({menu_container_, grid_renderer_});
    renderer_ = ftxui::Renderer(root_, [this] {
        return ftxui::hbox({
            render_sidebar(),
            ftxui::separator(),
            grid_renderer_->Render()
        }) | ftxui::border;
    });

    regenerate();
}

MazeApp::~MazeApp() {
    stop_solver();
}

void MazeApp::run() {
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
    screen_ = &screen;

    auto app = ftxui::CatchEvent(renderer_, [this](const ftxui::Event& event) {
        return handle_event(event);
    });

    screen.Loop(app);
    screen_ = nullptr;
    stop_solver();
}

void MazeApp::regenerate() {
    if (solving_.load()) return;
    std::lock_guard<std::mutex> lock(state_mutex_);
    maze_.generate(generator_values_[generator_index_], wall_cell_, passage_cell_);
    apply_terrain();
    visited_.clear();
    frontier_.clear();
    solution_index_.clear();
    solution_cells_.clear();
    show_solution_ = false;
    pulse_index_ = 0;
    current_cell_.reset();
    start_ = find_first_passage(false);
    dest_ = find_first_passage(true);
    cursor_ = start_;
}

void MazeApp::begin_solve() {
    if (solving_.load()) return;
    stop_solver();
    solving_ = true;
    stop_requested_ = false;

    solver_thread_ = std::thread([this] {
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            visited_.clear();
            frontier_.clear();
            solution_index_.clear();
            solution_cells_.clear();
            show_solution_ = false;
            pulse_index_ = 0;
            current_cell_.reset();
        }

        ExploreCallback callback = [this](const Cell& current,
            const std::vector<Cell>& frontier_cells,
            const std::unordered_set<Cell>& visited_cells) {
            if (stop_requested_.load()) return;
            std::lock_guard<std::mutex> lock(state_mutex_);
            current_cell_ = current;
            visited_ = visited_cells;
            frontier_.clear();
            frontier_.insert(frontier_cells.begin(), frontier_cells.end());
            if (screen_) screen_->PostEvent(ftxui::Event::Custom);
            std::this_thread::sleep_for(kExploreDelay);
        };

        Path path = maze_.findPath(
            algorithm_values_[algorithm_index_], start_, dest_, callback);

        if (!path.empty() && !stop_requested_.load()) {
            auto cells = build_cell_path(path);
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                solution_cells_ = cells;
                rebuild_solution_index();
                show_solution_ = true;
            }

            for (std::size_t i = 0; i < cells.size(); ++i) {
                if (stop_requested_.load()) break;
                {
                    std::lock_guard<std::mutex> lock(state_mutex_);
                    pulse_index_ = i;
                }
                if (screen_) screen_->PostEvent(ftxui::Event::Custom);
                std::this_thread::sleep_for(kPulseDelay);
            }

            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                pulse_index_ = cells.size();
            }
        }

        solving_ = false;
        if (screen_) screen_->PostEvent(ftxui::Event::Custom);
    });
}

void MazeApp::stop_solver() {
    stop_requested_ = true;
    if (solver_thread_.joinable()) {
        solver_thread_.join();
    }
}

void MazeApp::rebuild_solution_index() {
    solution_index_.clear();
    for (std::size_t i = 0; i < solution_cells_.size(); ++i) {
        solution_index_[solution_cells_[i]] = i;
    }
}

bool MazeApp::handle_event(const ftxui::Event& event) {
    if (event == ftxui::Event::Tab) {
        focus_on_grid_ = !focus_on_grid_;
        return true;
    }

    if (event == ftxui::Event::Character('q')
        || event == ftxui::Event::Character('Q')) {
        stop_requested_ = true;
        if (screen_) screen_->Exit();
        return true;
    }

    if (focus_on_grid_) {
        return handle_grid_event(event);
    }
    return handle_menu_event(event);
}

bool MazeApp::handle_grid_event(const ftxui::Event& event) {
    if (event == ftxui::Event::ArrowUp && cursor_.row > 0) {
        cursor_.row--;
        return true;
    }
    if (event == ftxui::Event::ArrowDown && cursor_.row + 1 < height_) {
        cursor_.row++;
        return true;
    }
    if (event == ftxui::Event::ArrowLeft && cursor_.col > 0) {
        cursor_.col--;
        return true;
    }
    if (event == ftxui::Event::ArrowRight && cursor_.col + 1 < width_) {
        cursor_.col++;
        return true;
    }
    if (event == ftxui::Event::Character('s')
        || event == ftxui::Event::Character('S')) {
        if (!maze_.at_unchecked(cursor_).wall) {
            start_ = cursor_;
        }
        return true;
    }
    if (event == ftxui::Event::Character('d')
        || event == ftxui::Event::Character('D')) {
        if (!maze_.at_unchecked(cursor_).wall) {
            dest_ = cursor_;
        }
        return true;
    }
    if (event == ftxui::Event::Character('r')
        || event == ftxui::Event::Character('R')) {
        regenerate();
        return true;
    }
    if (event == ftxui::Event::Character(' ')) {
        begin_solve();
        return true;
    }
    return false;
}

bool MazeApp::handle_menu_event(const ftxui::Event& event) {
    bool handled = menu_container_->OnEvent(event);
    if (terrain_index_ != last_terrain_index_) {
        last_terrain_index_ = terrain_index_;
        regenerate();
    }
    return handled;
}

ftxui::Element MazeApp::render_grid() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::vector<ftxui::Element> rows;
    rows.reserve(height_);

    for (std::size_t r = 0; r < height_; ++r) {
        std::vector<ftxui::Element> cells;
        cells.reserve(width_);
        for (std::size_t c = 0; c < width_; ++c) {
            Cell cell{r, c};
            const auto& meta = maze_.at_unchecked(cell);
            bool is_wall = meta.wall;
            bool is_cursor = focus_on_grid_ && (cell == cursor_);
            bool is_start = (cell == start_);
            bool is_dest = (cell == dest_);
            bool is_frontier = frontier_.contains(cell);
            bool is_visited = visited_.contains(cell);
            bool is_current = current_cell_.has_value() && (cell == *current_cell_);

            std::string glyph = is_wall ? "█" : "·";
            ftxui::Color fg = is_wall ? ftxui::Color::RGB(70, 70, 70)
                                      : map_color(meta.color);

            if (!is_wall && is_visited) {
                glyph = "•";
                fg = ftxui::Color::RGB(90, 170, 255);
            }
            if (!is_wall && is_frontier) {
                glyph = "•";
                fg = ftxui::Color::RGB(255, 210, 80);
            }
            if (!is_wall && is_current) {
                glyph = "●";
                fg = ftxui::Color::RGB(255, 255, 255);
            }

            if (show_solution_) {
                auto it = solution_index_.find(cell);
                if (it != solution_index_.end()) {
                    std::size_t idx = it->second;
                    if (idx < pulse_index_) {
                        glyph = "•";
                        fg = ftxui::Color::RGB(60, 220, 140);
                    } else if (idx == pulse_index_) {
                        glyph = "●";
                        fg = ftxui::Color::RGB(255, 255, 255);
                    } else if (idx == pulse_index_ + 1) {
                        glyph = "•";
                        fg = ftxui::Color::RGB(255, 230, 140);
                    } else if (idx == pulse_index_ + 2) {
                        glyph = "•";
                        fg = ftxui::Color::RGB(220, 190, 120);
                    }
                }
            }

            if (is_start) {
                glyph = "S";
                fg = ftxui::Color::RGB(80, 240, 160);
            }
            if (is_dest) {
                glyph = "D";
                fg = ftxui::Color::RGB(255, 110, 110);
            }

            auto cell_el = ftxui::text(glyph) | ftxui::color(fg);
            if (is_cursor) cell_el = cell_el | ftxui::inverted;
            cells.push_back(cell_el);
        }
        rows.push_back(ftxui::hbox(cells));
    }
    return ftxui::vbox(rows) | ftxui::border;
}

ftxui::Element MazeApp::render_sidebar() {
    std::string focus_label = focus_on_grid_
        ? "Focus: Grid (Tab to switch)"
        : "Focus: Menu (Tab to switch)";
    std::string status = solving_.load() ? "Solving..." : "Ready";

    return ftxui::vbox({
        ftxui::text("Algorithm") | ftxui::bold,
        algorithm_menu_->Render(),
        ftxui::separator(),
        ftxui::text("Maze Generator") | ftxui::bold,
        generator_menu_->Render(),
        ftxui::separator(),
        ftxui::text("Terrain") | ftxui::bold,
        terrain_menu_->Render(),
        ftxui::separator(),
        ftxui::text("Status: " + status),
        ftxui::text("Start: (" + std::to_string(start_.row) + ", "
            + std::to_string(start_.col) + ")"),
        ftxui::text("Goal: (" + std::to_string(dest_.row) + ", "
            + std::to_string(dest_.col) + ")"),
        ftxui::separator(),
        ftxui::text("Controls") | ftxui::bold,
        ftxui::text("Arrows  Move cursor/menu"),
        ftxui::text("Enter   Select menu"),
        ftxui::text("S/D     Set start/goal"),
        ftxui::text("Space   Solve"),
        ftxui::text("R       Regenerate"),
        ftxui::text("Tab     Switch focus"),
        ftxui::text("Q       Quit"),
        ftxui::separator(),
        ftxui::text(focus_label)
    }) | ftxui::border;
}

void MazeApp::apply_terrain() {
    if (terrains_.empty()) return;
    const auto& preset = terrains_[terrain_index_];
    if (preset.passages.empty()) return;
    std::uniform_int_distribution<std::size_t> pick(0, preset.passages.size() - 1);

    for (std::size_t r = 0; r < height_; ++r) {
        for (std::size_t c = 0; c < width_; ++c) {
            Cell cell{r, c};
            if (maze_.at_unchecked(cell).wall) {
                maze_.at_unchecked(cell) = wall_cell_;
            } else {
                maze_.at_unchecked(cell) = preset.passages[pick(rng_)];
            }
        }
    }
}

Cell MazeApp::find_first_passage(bool from_end) const {
    if (!from_end) {
        for (std::size_t r = 0; r < height_; ++r) {
            for (std::size_t c = 0; c < width_; ++c) {
                if (!maze_.at_unchecked({r, c}).wall) return {r, c};
            }
        }
    } else {
        for (std::size_t r = height_; r-- > 0;) {
            for (std::size_t c = width_; c-- > 0;) {
                if (!maze_.at_unchecked({r, c}).wall) return {r, c};
            }
        }
    }
    return {0, 0};
}

std::vector<Cell> MazeApp::build_cell_path(const Path& path) const {
    std::vector<Cell> cells;
    cells.reserve(path.size() + 1);
    Cell current = start_;
    cells.push_back(current);
    for (Direction dir : path) {
        current.move(dir);
        cells.push_back(current);
    }
    return cells;
}

ftxui::Color MazeApp::map_color(::Color color) {
    switch (color) {
        case Color::black: return ftxui::Color::RGB(10, 10, 10);
        case Color::red: return ftxui::Color::RGB(220, 60, 60);
        case Color::green: return ftxui::Color::RGB(70, 200, 110);
        case Color::blue: return ftxui::Color::RGB(80, 160, 220);
        case Color::yellow: return ftxui::Color::RGB(230, 200, 90);
        case Color::cyan: return ftxui::Color::RGB(80, 200, 200);
        case Color::magenta: return ftxui::Color::RGB(210, 90, 180);
        case Color::white: return ftxui::Color::RGB(230, 230, 230);
        case Color::gray: return ftxui::Color::RGB(140, 140, 140);
    }
    return ftxui::Color::RGB(230, 230, 230);
}

}  // namespace maze::ui

#ifndef __FIGURE_TYPES_H
#define __FIGURE_TYPES_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "mplc/figure_params.h"
#include "mplc/canvas/canvas_params.h"
#include "renderer/UI/window/window_types.h"


// 
enum class figure_placement_t {
    TOP_LEFT    = 0,
    CENTER      = 1,
    CUSTOM      = 2,
};

// forward decl
class figure_t;

// 
class figure_handle_t
{
public:
    uint32_t id = 0;

    bool is_valid() const { return id != 0; }
    bool operator==(const figure_handle_t &_other) { return id == _other.id; }

    figure_t &get() const;

    // canvas creators
    void scatter(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="", scatter_params_t _p={});
    void scatter(const std::vector<float> &_y, const std::string &_id="", scatter_params_t _p={});
    void lineplot(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id="", lineplot_params_t _p={});
    void lineplot(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="", lineplot_params_t _p={});
    void lineplot(const std::vector<float> &_y, const std::string &_id="", lineplot_params_t _p={});
    void histogram(const std::vector<float> &_data, const std::string &_id="", histogram_params_t _p={});

    // data update
    void data(const std::vector<float> &_y, const std::string &_id="");
    void data(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="");
    void data(const std::vector<std::vector<float>> &_y, const std::string &_id="");
    void data(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id="");

    // render
    void render();
    void draw(window_handle_t &_window);

    // accessors
    void set_title(const std::string &_title);
    void set_x_lim(const glm::vec2   &_lim);
    void set_y_lim(const glm::vec2   &_lim);
    void set_placement(figure_placement_t _p);
    void set_placement_offset(const glm::vec2 &_offset);
    void enable_grid();
    void disable_grid();
    void fill_x(const glm::vec2 &_lim);
    void fill_y(const glm::vec2 &_lim);
    void disable_fill_x();
    void disable_fill_y();
};


#endif // __FIGURE_TYPES_H

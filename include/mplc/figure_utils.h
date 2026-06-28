#ifndef __FIGURE_UTILS_H
#define __FIGURE_UTILS_H


#include <math.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "utils/log.h"


// axis constraints
#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2


//-------------------------------------------------------------------------------
/* Calculation of quantiles from an iterable type. Presumes sorted data _x.
 */
template<typename T>
typename T::value_type quantile(const T& _x, float _q)
{
    assert(_q >= 0.0 && _q <= 1.0);
    const auto n  = _x.size();
    const auto id = (n - 1) * _q;
    const auto lo = floor(id);
    const auto hi = ceil(id);
    const auto qs = _x[lo];
    const auto h  = (id - lo);

    return (1.0 - h) * qs + h * _x[hi];
}

// 
template<typename T>
glm::vec2 iqr(const T& _x)
{
    glm::vec2 q = { quantile(_x, 0.25), quantile(_x, 0.75) };
    return q;
}

//-------------------------------------------------------------------------------
// nice_scale_t
// Computes visually pleasing ticks
//

// Helper struct for rendering tick labels.
struct tick_labels_t
{
    size_t count = 0;
    std::vector<std::string> labels;
    float max_label_width = 0.0f;
    float min_label_width = 0.0f;
};

struct nice_scale_t
{
    glm::vec2 lim;
    size_t max_ticks   = 15;
    float tick_spacing = 0.0f;
    float range        = 0.0f ;
    bool nice_scale    = true;
    bool set           = false;

    union
    {
        glm::vec2 nice_lim;
        struct { float lower_bound; float upper_bound; };
    };
    
    tick_labels_t tick_labels;

    // 
    nice_scale_t() {}
    nice_scale_t(const glm::vec2& _lim, bool _nice_scale) :
        lim(_lim), nice_scale(_nice_scale)
    { calculate(); }
    
    void calculate();
    float nice_num(float _range, bool _round);

    void set_lim(const glm::vec2& _lim)   { lim = _lim;                                  calculate(); }
    void set_lim(float _lo, float _hi)    { lim = { _lo, _hi };                          calculate(); }
    void set_max_ticks(size_t _max_ticks) { max_ticks = static_cast<size_t>(_max_ticks); calculate(); }

    void print(const std::string& _info="");

};

//-------------------------------------------------------------------------------
// range_converter_t
// Maps a value from one range [xy_lim] into another [plot_lim].
// Used to convert data coordinates into normalized [0..1] plot coordinates.
// 
//  Example:
//   float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
//   float y_plot_max = _fig_params->data_height;
//   range_converter converter(dataLimY, { y_plot_min, y_plot_max } );
//   y_pos_plot = converter.eval(y_value);
// 
struct range_converter_t
{
    float xy_range_inv  = -1.0f;
    float plot_range    = -1.0f;
    glm::vec2 xy_lim    = { -1.0f, -1.0f };
    glm::vec2 plot_lim  = { -1.0f, -1.0f };

    // 
    range_converter_t() {}
    range_converter_t(const glm::vec2& _xy_lim, const glm::vec2& _plot_lim) :
        xy_range_inv(1.0f / (_xy_lim[1] - _xy_lim[0])), 
        plot_range(_plot_lim[1] - _plot_lim[0]),
        xy_lim(_xy_lim), 
        plot_lim(_plot_lim)
    {}

    // 
    float eval(float _val) const { return ((_val - xy_lim[0]) * plot_range * xy_range_inv) + plot_lim[0]; }
    void update_xy_range()       { xy_range_inv = 1.0f / (xy_lim[1] - xy_lim[0]); }
    void update_plot_range()     { plot_range = plot_lim[1] - plot_lim[0]; }

    void print(const std::string& _info="");

};

//-------------------------------------------------------------------------------
// axes_t
// Owns the two nice_scale_t + range_converter_t pairs for X and Y.
// Initialized from a figure_params_t, updated when data limits change.
// 
struct figure_params_t; // forward decl

struct axes_t
{
    nice_scale_t scalers[2];         // X = 0, Y = 1
    range_converter_t converters[2];

    // 
    axes_t() {}
    axes_t(const figure_params_t &_fig_params);

    float eval_x(float _val) const { return converters[0].eval(_val); }
    float eval_y(float _val) const { return converters[1].eval(_val); }

    nice_scale_t& x_ticks() { return scalers[0]; }
    nice_scale_t& y_ticks() { return scalers[1]; }

    void set_x_lim(const glm::vec2& _x_lim, bool _x_nice_scale);
    void set_y_lim(const glm::vec2& _y_lim, bool _y_nice_scale);

    void print(const std::string& _info="");
    
};

//---------------------------------------------------------------------------
// marker geometry
// 
enum class figure_marker_t;     // forward decl (defined in figure_params.h)
struct normalized_params_t;     // forward decl

size_t figer_marker_vertex_count(figure_marker_t _marker);

// Fills _vertices with the geometry for a single marker centred at the
// origin. The caller adds a data-point offset to position it in the plot.
// Returns the vertex count (same as figure_marker_vertex_count).
size_t figure_marker_vertices(const normalized_params_t &_p,
                              std::vector<glm::vec2> &_vertices);



#endif // __FIGURE_UTILS_H

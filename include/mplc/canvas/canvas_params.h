#ifndef __CANVAS_PARAMS_H
#define __CANVAS_PARAMS_H

#include "mplc/figure_params.h"


//---------------------------------------------------------------------------
// per-canvas parameter structs
// Passed at canvas creation time, copied into the canvas's own
// figure_params_t via figure_params_t::set_from_*
// 
struct scatter_params_t
{
    figure_marker_t marker;
    float           marker_size;
    glm::vec4       marker_color;
    size_t          x_tick_count;
    size_t          y_tick_count;    
};

//
struct lineplot_params_t
{
    float           line_width_px;
    figure_marker_t marker;
    float           marker_size;
    glm::vec4       line_color;
    size_t          x_tick_count;
    size_t          y_tick_count;
    bool            x_nice_scale;
    bool            y_nice_scale;
};

//
struct histogram_params_t
{
    size_t          bin_count;
    bool            hist_line_plot;
};


#endif // __CANVAS_PARAMS_H

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
    figure_marker_t marker          = figure_marker_t::SQUARE;
    float           marker_size     = 7.0f;
    glm::vec4       marker_color    = glm::vec4(0.85f, 0.50f, 0.15f, 1.0f);
    size_t          x_tick_count    = 2;
    size_t          y_tick_count    = 2;    
};

//
struct lineplot_params_t
{
    float           line_width_px   = 1.0f;
    figure_marker_t marker          = figure_marker_t::NONE;
    float           marker_size     = 7.0f;
    glm::vec4       line_color      = glm::vec4(0.85f, 0.50f, 0.15f, 1.0f);
    size_t          x_tick_count    = 2;
    size_t          y_tick_count    = 2;
    bool            x_nice_scale    = true;
    bool            y_nice_scale    = true;
};

//
struct histogram_params_t
{
    size_t          bin_count       = -1;
    bool            hist_line_plot  = false;
    glm::vec4       bin_color       = glm::vec4(0.85f, 0.50f, 0.15f, 1.0f);
};


#endif // __CANVAS_PARAMS_H

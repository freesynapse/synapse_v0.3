
#include <cstdio>
#include <cmath>

#include "mplc/figure_params.h"
#include "mplc/canvas/canvas_params.h"

#include "utils/log.h"


// 
void normalized_params_t::set(const figure_params_t &_params)
{
    const float sx = _params.figure_sz_px.x;
    const float sy = _params.figure_sz_px.y;

    figure_type             = _params.figure_type;
    figure_sz_px            = _params.figure_sz_px;

    data_spacing            = _params.data_spacing;
    data_axis_offset        = norm2(_params.data_axis_offset_px, _params.figure_sz_px);
    z_value_data            = _params.z_value_data;
    z_value_aux             = _params.z_value_aux;

    canvas_origin           = norm2(_params.canvas_origin_px, _params.figure_sz_px);

    x_axis_lim[0]           = norm(_params.canvas_origin_px.x + _params.x_axis_lim_px[0], sx);
    x_axis_lim[1]           = norm_lim(_params.x_axis_lim_px[1], sx);
    x_nice_scale            = _params.x_nice_scale;
    x_axis_length           = x_axis_lim[1] - x_axis_lim[0];

    y_axis_lim[0]           = norm(_params.canvas_origin_px.y + _params.y_axis_lim_px[0], sy);
    y_axis_lim[1]           = norm_lim(_params.y_axis_lim_px[1], sy);
    y_nice_scale            = _params.y_nice_scale;
    y_axis_length           = y_axis_lim[1] - y_axis_lim[0];

    data_height             = norm(_params.canvas_origin_px.y + _params.data_height_px, sy);

    render_x_axis           = _params.render_x_axis;
    render_y_axis           = _params.render_y_axis;

    axes_neg_protrusion     = norm2(_params.axes_neg_protrusion_px, _params.figure_sz_px);

    tick_length             = norm2(_params.tick_length_px, _params.figure_sz_px);
    x_tick_count            = _params.x_tick_count;
    render_x_ticks          = _params.render_x_ticks;
    y_tick_count            = _params.y_tick_count;
    render_y_ticks          = _params.render_y_ticks;

    axis_label_font_size_px = _params.axis_label_font_size_px;
    render_axis_labels      = _params.render_axis_labels;
    tick_labels_offset      = norm2(_params.tick_labels_offset_px, _params.figure_sz_px);
    tick_label_font_size_px = _params.tick_label_font_size_px;
    render_ticklabels       = _params.render_ticklabels;
    rotation_x_ticklabels   = _params.rotation_x_ticklabels;
    rotation_y_ticklabels   = _params.rotation_y_ticklabels;

    render_title            = _params.render_title;
    title_font_size_px      = _params.title_font_size_px;

    axis_color              = _params.axis_color;
    tick_color              = _params.tick_color;
    title_color             = _params.title_color;
    axis_label_color        = _params.axis_label_color;
    tick_label_color        = _params.tick_label_color;
    figure_background       = _params.figure_background;
    data_color              = _params.data_color;
    fill_between_color      = _params.fill_between_color;

    bar_spacing             = norm(_params.bar_spacing_px, sx);
    bin_count               = _params.bin_count;
    hist_line_plot          = _params.hist_line_plot;

    marker_sz               = norm(_params.marker_sz_px, sy);
    marker_y_sz             = marker_sz;
    marker_x_sz             = marker_sz * (sy / sx);
    marker                  = _params.marker;
    line_width_px           = _params.line_width_px;

    fill_x                  = _params.fill_x;
    fill_y                  = _params.fill_y;
}

// 
void figure_params_t::set_from_scatter_params(const scatter_params_t &_p)
{
    figure_type     = figure_type_t::SCATTERPLOT;
    marker          = _p.marker;
    marker_sz_px    = _p.marker_size;
    x_tick_count    = _p.x_tick_count;
    y_tick_count    = _p.y_tick_count;
    if (_p.marker_color != glm::vec4(0.0f))
        data_color      = _p.marker_color;
}

// 
void figure_params_t::set_from_lineplot_params(const lineplot_params_t &_p)
{
    figure_type     = figure_type_t::LINEPLOT;
    marker          = _p.marker;
    marker_sz_px    = _p.marker_size;
    line_width_px   = _p.line_width_px;
    if (_p.line_color != glm::vec4(0.0f))
        data_color      = _p.line_color;
    x_tick_count    = _p.x_tick_count;
    y_tick_count    = _p.y_tick_count;
    x_nice_scale    = _p.x_nice_scale;
    y_nice_scale    = _p.y_nice_scale;
}

// 
void figure_params_t::set_from_histogram_params(const histogram_params_t &_p)
{
    figure_type     = figure_type_t::HISTOGRAM;
    bin_count       = _p.bin_count;
    hist_line_plot  = _p.hist_line_plot;
    if (_p.bin_color != glm::vec4(0.0f))
        data_color      = _p.bin_color;
}

// 
void figure_params_t::print_parameters() const
{
    SYN_DEBUG("-------- figure_params_t --------\n");
    SYN_DEBUG("  %-30s : %s\n",           "figure_type",                figure_type_str(figure_type));
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "figure_sz_px",               figure_sz_px.x, figure_sz_px.y);
    SYN_DEBUG("  %-30s : %.2f\n",         "data_height_px",             data_height_px);
    SYN_DEBUG("  %-30s : %.2f\n",         "data_spacing",               data_spacing);
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "data_axis_offset_px",        data_axis_offset_px.x, data_axis_offset_px.y);
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "canvas_origin_px",           canvas_origin_px.x, canvas_origin_px.y);
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "x_axis_lim_px",              x_axis_lim_px.x, x_axis_lim_px.y);
    SYN_DEBUG("  %-30s : %s\n",           "x_nice_scale",               x_nice_scale ? "true" : "false");
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "y_axis_lim_px",              y_axis_lim_px.x, y_axis_lim_px.y);
    SYN_DEBUG("  %-30s : %s\n",           "y_nice_scale",               y_nice_scale ? "true" : "false");
    SYN_DEBUG("  %-30s : %s\n",           "render_x_axis",              render_x_axis ? "true" : "false");
    SYN_DEBUG("  %-30s : %s\n",           "render_y_axis",              render_y_axis ? "true" : "false");
    SYN_DEBUG("  %-30s : (%.2f, %.2f)\n", "tick_length_px",             tick_length_px.x, tick_length_px.y);
    SYN_DEBUG("  %-30s : %zu\n",          "x_tick_count",               x_tick_count);
    SYN_DEBUG("  %-30s : %zu\n",          "y_tick_count",               y_tick_count);
    SYN_DEBUG("  %-30s : %s\n",           "render_tick_labels",         render_ticklabels ? "true" : "false");
    SYN_DEBUG("  %-30s : %.2f\n",         "tick_label_font_size_px",    tick_label_font_size_px);
    SYN_DEBUG("  %-30s : %s\n",           "render_title",               render_title ? "true" : "false");
    SYN_DEBUG("  %-30s : %.2f\n",         "title_font_size_px",         title_font_size_px);
    SYN_DEBUG("  %-30s : (%.2f,%.2f,%.2f,%.2f)\n", "bg_color",          figure_background.r, figure_background.g, figure_background.b, figure_background.a);
    SYN_DEBUG("  %-30s : (%.2f,%.2f,%.2f,%.2f)\n", "data_color",        data_color.r, data_color.g, data_color.b, data_color.a);
    SYN_DEBUG("  %-30s : %s\n",           "fill_x",                     fill_x ? "true" : "false");
    SYN_DEBUG("  %-30s : %s\n",           "fill_y",                     fill_y ? "true" : "false");

    if (figure_type == figure_type_t::HISTOGRAM)
    {
        SYN_DEBUG("  %-30s : %.2f\n",  "bar_spacing_px",  bar_spacing_px);
        SYN_DEBUG("  %-30s : %d\n",    "bin_count",        bin_count);
        SYN_DEBUG("  %-30s : %s\n",    "hist_line_plot",   hist_line_plot ? "true" : "false");
    }
    if (figure_type == figure_type_t::SCATTERPLOT ||
        figure_type == figure_type_t::LINEPLOT)
    {
        SYN_DEBUG("  %-30s : %.2f\n",  "marker_sz_px",    marker_sz_px);
        SYN_DEBUG("  %-30s : %s\n",    "marker",           figure_marker_str(marker));
    }
    if (figure_type == figure_type_t::LINEPLOT)
        SYN_DEBUG("  %-30s : %.2f\n",  "line_width_px",   line_width_px);
}

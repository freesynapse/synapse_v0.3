#ifndef __FIGURE_PARAMS_H
#define __FIGURE_PARAMS_H

#include <glm/glm.hpp>

// forward decls from canvas_2d
struct scatter_params_t;
struct lineplot_params_t;
struct histogram_params_t;

// 
enum class figure_type_t
{
    NONE        = 0,
    HISTOGRAM   = 1,
    LINEPLOT    = 2,
    SCATTERPLOT = 3,
    STEMPLOT    = 4,
};

// 
static inline const char *figure_type_str(figure_type_t _type)
{
    switch(_type)
    {
    case figure_type_t::NONE:          return "NONE";
    case figure_type_t::HISTOGRAM:     return "HISTOGRAM";
    case figure_type_t::LINEPLOT:      return "LINEPLOT";
    case figure_type_t::SCATTERPLOT:   return "SCATTERPLOT";
    case figure_type_t::STEMPLOT:      return "STEMPLOT";
    default:                        return "unknown figure_type_t";
    }
}

// figure_marker_t -- used for markers in scatter plots and lineplots
enum class figure_marker_t
{
    SQUARE      = 0,
    DIAMOND     = 1,
    TRI_DOWN    = 2,
    TRI_UP      = 3,
    HLINE       = 4,
    VLINE       = 5,
    PLUS        = 6,
    DOT         = 7,
    NONE        = 8,
};

// 
static inline const char *figure_marker_str(figure_marker_t _marker)
{
    switch(_marker)
    {
        case figure_marker_t::SQUARE:   return "SQUARE";
        case figure_marker_t::DIAMOND:  return "DIAMOND";
        case figure_marker_t::TRI_DOWN: return "TRI_DOWN";
        case figure_marker_t::TRI_UP:   return "TRI_UP";
        case figure_marker_t::HLINE:    return "HLINE";
        case figure_marker_t::VLINE:    return "VLINE";
        case figure_marker_t::PLUS:     return "PLUS";
        case figure_marker_t::DOT:      return "DOT";
        case figure_marker_t::NONE:     return "NONE";
        default:                        return "unknown figure_marker_t";
    }
}

//-------------------------------------------------------------------------------
// figure_params_t -- all figure parameters and some more
// 
struct figure_params_t
{
    // Type of plot, used for rendering options. Not set by user but by 
    // derivations of FigureBase and FigureRenderObjBase.
    figure_type_t figure_type           = figure_type_t::NONE;

    // Size of the canvas, used for normalization of coordinates. Set during 
    // construction.
    glm::vec2 figure_sz_px              = glm::vec2(420.0f, 280.0f);
    
    // Height of highest data point in pixel-space; if below 0, treated as 
    // negaive offset from y_axis_lim_px[1].
    float data_height_px                = 0;
                                
    // Space between adjecent data points.
    float data_spacing                  = -1;

    // Offset from axes in pixels, for X only first and last data point, and 
    // for Y for all data points.
    glm::vec2 data_axis_offset_px       = { 0, 0 };

    // Data z axis value.
    float z_value_data                  = 0.0f;

    // Plot auxillary z value (axes, gridlines)
    float z_value_aux                   = 0.0f;

    // Negative offset in pixels, the amount of backward 'protrusion' of axes 
    glm::vec2 axes_neg_protrusion_px    = { 0, 0 };

    // Canvas origin in pixel space.
    glm::vec2 canvas_origin_px          = { 80, 50 };

    // The actual X axis coordinates (pixels). Values represent
    // [0] :  offset from canvas_origin_px.x, both negative and positive values
    // [1] :  offset from canvas_sz.x for negative values and offset from 
    // canvas_origin_px.x for positive values 
    glm::vec2 x_axis_lim_px             = { 0, -30 };

    // Flag to determine the scaler of the X axis: should the axis extend to the
    // last data point (X-coordinate) or should a NiceScale be applied?
    bool x_nice_scale                   = true;
    
    // X axis length -- automatically calculated after normalization.
    float x_axis_length                 = 0.0f;
    
    // The actual Y axis coordinates (pixels). Values represent
    // [0] :  offset from canvas_origin_px.y
    // [1] :  offset from canvas_sz.y for negative values and offset from 
    // canvas_origin_px.y for positive values 
    glm::vec2 y_axis_lim_px             = { 0, -30 }; 

    // Flag to determine the scaler of the Y axis: should the axis extend to the
    // last data point (Y-coordinate) or should a NiceScale be applied?
    bool y_nice_scale                   = true;

    // Y axis length -- automatically calculated after normalization.
    float y_axis_length                 = 0.0f;
    
    // Render X axis?
    bool render_x_axis                  = true;

    // Render Y axis?
    bool render_y_axis                  = true;

    // Length of ticks in pixels, specified separately for each axis.
    glm::vec2 tick_length_px            = { 10, 10 };

    // Number of X ticks.
    size_t x_tick_count                 = 2;

    // Render X axis ticks?
    bool render_x_ticks                 = true;

    // Number of Y ticks.
    size_t y_tick_count                 = 2;

    // Render Y axis ticks?
    bool render_y_ticks                 = true;

    // Font size of axis labels
    float axis_label_font_size_px       = 12.0f;

    // Render axis labels?
    bool render_axis_labels             = true;

    // Ticklabels offset from the lower tick bound, for each axis, respectively.
    glm::vec2 tick_labels_offset_px     = { 5, 10 };

    // Font size of ticklabels.
    float tick_label_font_size_px       = 12.0f;

    // Render ticklabels?
    bool render_ticklabels              = true;

    // Rotation of X axis ticklabels, clock-wise, in degrees.
    int rotation_x_ticklabels           = 0;

    // Rotation of Y axis ticklabels, clock-wise, in degrees.
    int rotation_y_ticklabels           = 0;

    // Is there a plot title? 
    bool render_title                   = true;

    // Font size of plot title
    float title_font_size_px            = 16.0f;

    // Matches the default ImGui palette.
    glm::vec4 tick_color                = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 axis_color                = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 title_color               = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 axis_label_color          = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 tick_label_color          = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 figure_background         = {   0.0f,   0.0f,   0.0f,  0.7f };
    glm::vec4 data_color                = { 0.298f, 0.361f, 0.490f,  1.0f };
    glm::vec4 fill_between_color        = {   1.0f,   1.0f,   1.0f,  0.2f };


    // histogram parameters
    
    // Space between bars in pixels.
    float bar_spacing_px                = 1;

    // Number of bins.
    int bin_count                       = -1;

    // Line plot instead of bars?
    bool hist_line_plot                 = false;

    
    // scatter plot parameters

    // Size of markers, in pixels on the Y axis. Markers X size is calculated
    // from the X/Y aspect ratio upon normalization.
    float marker_sz_px                  = 4.0f;

    // Marker type, as defined above.
    figure_marker_t marker              = figure_marker_t::SQUARE;

    
    // lineplot parameters
    
    // Line width in pixels (as set by OpenGL).
    float line_width_px                 = 1.0f;


    // selection (fill) parameters

    // Render X selection (i.e. called fill_between_x).
    bool fill_x                         = false;

    // Render Y selection (i.e. called fill_between_y).
    bool fill_y                         = false;

    // Constructors
    figure_params_t() = default;
    figure_params_t(const glm::vec2& _fig_sz_px) : figure_sz_px(_fig_sz_px) {}

    //  Update relevant fields from canvas parameters
    void set_from_scatter_params(const scatter_params_t& _params);
    void set_from_lineplot_params(const lineplot_params_t& _params);
    void set_from_histogram_params(const histogram_params_t& _params);

    // Debug (somewhat)
    void print_parameters() const;
    
};

// Takes a figure_params_t object and normalizes all pixel values to normalized
// coordinates [0..1] for rendering. The shader then converts normalized 
// coordinates to normalized device coordinates (NDC). Fields already in 
// normalized coordinates and non-coordinate parameters will simply be copied.
// 
struct normalized_params_t
{
    figure_type_t   figure_type;
    glm::vec2       figure_sz_px;
    
    float           data_height;
    float           data_spacing;
    glm::vec2       data_axis_offset;
    float           z_value_data;
    float           z_value_aux;

    glm::vec2       canvas_origin;
    glm::vec2       x_axis_lim;
    bool            x_nice_scale;
    float           x_axis_length;
    glm::vec2       y_axis_lim;
    bool            y_nice_scale;
    float           y_axis_length;
    bool            render_x_axis;
    bool            render_y_axis;
    
    glm::vec2       axes_neg_protrusion;

    glm::vec2       tick_length;
    size_t          x_tick_count;
    bool            render_x_ticks;
    size_t          y_tick_count;
    bool            render_y_ticks;
    float           axis_label_font_size_px;
    bool            render_axis_labels;
    glm::vec2       tick_labels_offset;
    float           tick_label_font_size_px;
    bool            render_ticklabels;
    int             rotation_x_ticklabels;
    int             rotation_y_ticklabels;
    bool            render_title;
    float           title_font_size_px;

    glm::vec4       axis_color;
    glm::vec4       tick_color;
    glm::vec4       title_color;
    glm::vec4       axis_label_color;
    glm::vec4       tick_label_color;
    glm::vec4       figure_background;
    glm::vec4       data_color;
    glm::vec4       fill_between_color;

    // Histogram parameters
    float           bar_spacing;
    int             bin_count;
    bool            hist_line_plot;

    // Scatter plot parameters
    float           marker_sz;
    float           marker_y_sz;
    float           marker_x_sz;
    figure_marker_t marker;

    // Lineplot parameters
    float           line_width_px;

    // Selection (fill) parameters
    bool            fill_x;
    bool            fill_y;

    //
    normalized_params_t() = default;
    normalized_params_t(const figure_params_t &_params) { set(_params); }
    normalized_params_t(const figure_params_t *_params) { set(*_params); }

    void set(const figure_params_t &_params);

private:
    inline float norm(float _px, float _axis_sz) const { return _px / _axis_sz; }
    inline float norm_lim(float _lim, float _axis_sz) const { return (_lim < 0.0f) ? norm(_axis_sz + _lim, _axis_sz) : norm(_lim, _axis_sz); }
    inline glm::vec2 norm_lim2(const glm::vec2 &_v, float _axis_sz) const { return { norm_lim(_v[0], _axis_sz), norm_lim(_v[1], _axis_sz) }; }
    inline glm::vec2 norm2(const glm::vec2 &_v, const glm::vec2 &_ax) const { return { norm(_v[0], _ax.x), norm(_v[1], _ax.y) }; }
    
};

// global (namespace) instance
extern figure_params_t rc_params;


#endif // __FIGURE_PARAMS_H

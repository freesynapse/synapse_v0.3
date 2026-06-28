
#include <stdio.h>
#include <math.h>
#include <limits>

#include "mplc/figure_utils.h"

#include "mplc/figure_params.h"


//-------------------------------------------------------------------------------
// nice_scale_t
//
void nice_scale_t::calculate()
{
    if (nice_scale)
        range = nice_num(lim[1] - lim[0], false);
    else
        range = lim[1] - lim[0];

    tick_spacing = nice_num(range / (static_cast<float>(max_ticks) - 1.0f), true);

    if (nice_scale)
    {
        nice_lim[0] = floor(lim[0] / tick_spacing) * tick_spacing;
        nice_lim[1] = ceil(lim[1] / tick_spacing) * tick_spacing;
    }
    else
    {
        nice_lim[0] = (lim[0] / tick_spacing) * tick_spacing;
        nice_lim[1] = (lim[1] / tick_spacing) * tick_spacing;
    }
    
    // update parameters
    max_ticks = static_cast<size_t>((upper_bound - lower_bound) / tick_spacing);
    if (nice_lim[0] == 0.0f)
        max_ticks++;
    range = nice_lim[1] - nice_lim[0];
    set = true;
}

//
float nice_scale_t::nice_num(float _range, bool _round)
{
    float exponent;
    float fraction;
    float nice_fraction;

    exponent = floor(log10(_range));
    fraction = _range / pow(10.f, exponent);

    if (_round) 
    {   if (fraction < 1.5)
            nice_fraction = 1;
        else if (fraction < 3)
            nice_fraction = 2;
        else if (fraction < 7)
            nice_fraction = 5;
        else
            nice_fraction = 10;
    } 
    else 
    {   if (fraction <= 1)
            nice_fraction = 1;
        else if (fraction <= 2)
            nice_fraction = 2;
        else if (fraction <= 5)
            nice_fraction = 5;
        else
            nice_fraction = 10;
    }
    return nice_fraction * pow(10, exponent);                
}

// 
void nice_scale_t::print(const std::string& _info)
{
    SYN_DEBUG("------------------------- nice_scale_t -------------------------\n");
    if (_info != "")
        SYN_DEBUG("    %s\n", _info.c_str());
    SYN_DEBUG("    size_t  max_ticks         =    %zu\n",  max_ticks);
    SYN_DEBUG("    float   tick_spacing      =    %.2f\n", tick_spacing);
    SYN_DEBUG("    float   range             =    %.2f\n", range);
    SYN_DEBUG("    float   lower_bound       =    %.2f\n", lower_bound);
    SYN_DEBUG("    float   upper_bound       =    %.2f\n", upper_bound);
    
}

//-------------------------------------------------------------------------------
// range_converter_t
// 
void range_converter_t::print(const std::string& _info)
{
    SYN_DEBUG("------------------------- range_converter_t -------------------------\n");
    if (_info != "")
        SYN_DEBUG("%s\n", _info.c_str());
    SYN_DEBUG("    vec2  xy_lim              =    %.2f, %.2f\n", xy_lim[0], xy_lim[1]);
    SYN_DEBUG("    float xy_range            =    %.2f\n", 1.0f / xy_range_inv);
    SYN_DEBUG("    float xy_range_inv        =    %.2f\n", xy_range_inv);
    SYN_DEBUG("    vec2  plot_lim            =    %.2f, %.2f\n", plot_lim[0], plot_lim[1]);
    SYN_DEBUG("    float plot_range          =    %.2f\n", plot_range);

}

//-------------------------------------------------------------------------------
// axes_t
//
axes_t::axes_t(const figure_params_t &_fig_params)
{
    normalized_params_t params(_fig_params);
    
    converters[0].plot_lim = { params.x_axis_lim[0], params.x_axis_lim[1] };
    converters[0].update_plot_range();
    
    converters[1].plot_lim = { params.y_axis_lim[0], params.y_axis_lim[1] };
    converters[1].update_plot_range();
    
}

//
void axes_t::set_x_lim(const glm::vec2& _x_lim, bool _x_nice_scale)
{
    scalers[0] = nice_scale_t(_x_lim, _x_nice_scale);
    converters[0].xy_lim = scalers[0].nice_lim;
    converters[0].update_xy_range();
    
}

//
void axes_t::set_y_lim(const glm::vec2& _y_lim, bool _y_nice_scale)
{
    scalers[1] = nice_scale_t(_y_lim, _y_nice_scale);
    converters[1].xy_lim = scalers[1].nice_lim;
    converters[1].update_xy_range();
    
}

// 
void axes_t::print(const std::string& _info)
{
    SYN_DEBUG("-------------------- AXES OBJECT ---------------------\n");
    if (_info != "")
        SYN_DEBUG("%s\n", _info.c_str());
    scalers[0].print("X_AXIS");
    converters[0].print("X CONVERTER");
    scalers[1].print("Y_AXIS");
    converters[1].print("Y CONVERTER");
    
}

//-------------------------------------------------------------------------------
// marker geometry
// 
size_t figer_marker_vertex_count(figure_marker_t _marker)
{
    switch (_marker)
    {
        case figure_marker_t::SQUARE:   return  6;
        case figure_marker_t::DIAMOND:  return  6;
        case figure_marker_t::TRI_DOWN: return  3;
        case figure_marker_t::TRI_UP:   return  3;
        case figure_marker_t::HLINE:    return  6;
        case figure_marker_t::VLINE:    return  6;
        case figure_marker_t::PLUS:     return 12;
        case figure_marker_t::DOT:      return  0;  // TODO : implement this
        case figure_marker_t::NONE:     return  0;
        default:                        return  0;
    }
    return 0;
}

//
size_t figure_marker_vertices(const normalized_params_t  &_params,
                              std::vector<glm::vec2>& _vertices)
{
    _vertices.clear();
    figure_marker_t marker = _params.marker;
    
    if (marker == figure_marker_t::NONE)
        return 0;

    const float x_m_sz = _params.marker_x_sz;
    const float y_m_sz = _params.marker_y_sz;

    static float h = 1.0f;
    static float s = sqrt(1.5 * h);
    glm::vec2 A, B, C;

    switch (marker)
    {
        case figure_marker_t::SQUARE:
            
            _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
            _vertices.push_back({  0.5f * x_m_sz, -0.5f * y_m_sz });
            _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
            _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
            _vertices.push_back({ -0.5f * x_m_sz,  0.5f * y_m_sz });
            _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
            break;
    
        //
        case figure_marker_t::DIAMOND:
            _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
            _vertices.push_back({  0.000f * 1.2f * x_m_sz, -0.707f * 1.2f * y_m_sz });
            _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
            _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
            _vertices.push_back({  0.000f * 1.2f * x_m_sz,  0.707f * 1.2f * y_m_sz });
            _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
            break;
        //
        case figure_marker_t::TRI_DOWN:
            A = { -h*0.5f,    s-(h*0.5f) };
            B = {  h*0.5f,    s-(h*0.5f) };
            C = {    0.0f, 0.0f-(h*0.5f) };
            _vertices.push_back({ C[0] * 1.5f * x_m_sz, C[1] * 1.5f * y_m_sz });
            _vertices.push_back({ B[0] * 1.5f * x_m_sz, B[1] * 1.5f * y_m_sz });
            _vertices.push_back({ A[0] * 1.5f * x_m_sz, A[1] * 1.5f * y_m_sz });
            break;
        //
        case figure_marker_t::TRI_UP:
            A = { -h*0.5f, 0.0f-(h*0.5f) };
            B = {  h*0.5f, 0.0f-(h*0.5f) };
            C = {    0.0f,    s-(h*0.5f) };
            _vertices.push_back({ A[0] * 1.5f * x_m_sz, A[1] * 1.5f * y_m_sz });
            _vertices.push_back({ B[0] * 1.5f * x_m_sz, B[1] * 1.5f * y_m_sz });
            _vertices.push_back({ C[0] * 1.5f * x_m_sz, C[1] * 1.5f * y_m_sz });
            break;
        //
        case figure_marker_t::HLINE:
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            break;
        //
        case figure_marker_t::VLINE:
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            break;
        //
        case figure_marker_t::PLUS:
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
            _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
            break;
        //
        default:
            SYN_WARNING("invalid figure_marker_t (%d).\n", (uint32_t)marker);
            return 0;
            break;
    }
    //
    return _vertices.size();
}

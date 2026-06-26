
#include "mplc/canvas/canvas_2d.h"
#include "mplc/canvas/scatter_plot_2d.h"
#include "mplc/canvas/line_plot_2d.h"
#include "mplc/canvas/histogram_2d.h"


// 
static const char *canvas_type_str(canvas_type_t _type)
{
    switch (_type) {
        case canvas_type_t::SCATTER:    return "SCATTER";
        case canvas_type_t::LINEPLOT:   return "LINEPLOT";
        case canvas_type_t::HISTOGRAM:  return "HISTOGRAM";
        default: return "(unknown canvas_type_t)";
    }
    
}

// 
void canvas_finalize_data(canvas_2d_t &_c)
{
    switch (_c.type) {
        case canvas_type_t::SCATTER:    scatter_finalize_data(_c);      break;
        case canvas_type_t::LINEPLOT:   lineplot_finalize_data(_c);     break;
        case canvas_type_t::HISTOGRAM:  histogram_finalize_data(_c);    break;    
    }

}

// 
void canvas_data(canvas_2d_t &_c, const std::vector<float> &_y)
{
    switch (_c.type) {
        case canvas_type_t::SCATTER:    scatter_data(_c, _y);       break;
        case canvas_type_t::LINEPLOT:   lineplot_data(_c, _y);      break;
        case canvas_type_t::HISTOGRAM:  histogram_data(_c, _y);     break;    
    }
    
}

// 
void canvas_data(canvas_2d_t &_c, const std::vector<float> &_x,
                                  const std::vector<float> &_y)
{
    switch (_c.type) {
        case canvas_type_t::SCATTER:    scatter_data(_c, _x, _y);   break;
        case canvas_type_t::LINEPLOT:   lineplot_data(_c, _x, _y);  break;
        default: SYN_WARNING("x, y: not supported for canvas type %s.\n", canvas_type_str(_c.type)); break;
    }
    
}

// 
void canvas_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_y)
{
    switch (_c.type) {
        case canvas_type_t::LINEPLOT:   lineplot_data(_c, _y);  break;
        default: SYN_WARNING("x, y: not supported for canvas type %s.\n", canvas_type_str(_c.type)); break;
    }

}

// 
void canvas_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_x,
                                  const std::vector<std::vector<float>> &_y)
{
    switch (_c.type) {
        case canvas_type_t::LINEPLOT:   lineplot_data(_c, _x, _y);  break;
        default: SYN_WARNING("x, y: not supported for canvas type %s.\n", canvas_type_str(_c.type)); break;
    }
    
}

// 
void canvas_redraw(canvas_2d_t &_c, const axes_t &_axes)
{
    switch (_c.type) {
        case canvas_type_t::SCATTER:    scatter_redraw(_c, _axes);      break;
        case canvas_type_t::LINEPLOT:   lineplot_redraw(_c, _axes);     break;
        case canvas_type_t::HISTOGRAM:  histogram_redraw(_c, _axes);    break;    
    }

}

// 
void canvas_render(const canvas_2d_t &_c, shader_t &_shader)
{
    switch (_c.type) {
        case canvas_type_t::SCATTER:    scatter_render(_c, _shader);    break;
        case canvas_type_t::LINEPLOT:   lineplot_render(_c, _shader);   break;
        case canvas_type_t::HISTOGRAM:  histogram_render(_c, _shader);  break;    
    }
    
}

// 
void canvas_destroy(canvas_2d_t &_c)
{
    _c.vao_data.destroy();
    if (_c.type == canvas_type_t::LINEPLOT) _c.lineplot.vao_markers.destroy();

    // destroy active union members
    switch (_c.type) {
        case canvas_type_t::SCATTER:    _c.scatter.~scatter_data_t();       break;
        case canvas_type_t::LINEPLOT:   _c.lineplot.~lineplot_data_t();     break;
        case canvas_type_t::HISTOGRAM:  _c.histogram.~histogram_data_t();   break;    
    }
    
}

// 
std::string canvas_resolve_id(const std::vector<std::unique_ptr<canvas_2d_t>> &_canvases,
                              const std::string &_id)
{
    std::string id = _id.empty() ? "canvas" : _id;
    int suffix = 1;
    std::string candidate = id;
    while (true) {
        bool found = false;
        for (const auto &c : _canvases) {
            if (c->id == candidate) { 
                found = true; 
                break;
            }
        }
        if (!found) break;
        candidate = id + "_" + std::to_string(suffix++);
    }
    return candidate;
    
}

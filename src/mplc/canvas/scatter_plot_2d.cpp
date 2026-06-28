
#include "mplc/canvas/scatter_plot_2d.h"

#include "mplc/figure_utils.h"
#include "utils/log.h"

#include "c_api.h"


// 
void scatter_finalize_data(canvas_2d_t &_c)
{
    scatter_data_t &s = _c.scatter;
    SYN_ASSERT(s.x.size() == s.y.size(), "scatter: x and y must have equal length.");
    _c.data_lim_x = { std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::lowest() };
    _c.data_lim_y = { std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::lowest() };

    for (size_t i = 0; i < s.x.size(); i++) {
        _c.data_lim_x[0] = std::min(_c.data_lim_x[0], s.x[i]);
        _c.data_lim_x[1] = std::max(_c.data_lim_x[1], s.x[i]);
        _c.data_lim_y[0] = std::min(_c.data_lim_y[0], s.y[i]);
        _c.data_lim_y[1] = std::max(_c.data_lim_y[1], s.y[i]);        
    }
}

// 
void scatter_data(canvas_2d_t &_c, const std::vector<float> &_y)
{
    std::vector<float> x(_y.size());
    for (size_t i = 0; i < _y.size(); i++) {
        x[i] = (float)i;
    }
    scatter_data(_c, x, _y);
}

// 
void scatter_data(canvas_2d_t &_c, const std::vector<float> &_x, const std::vector<float> &_y)
{
    _c.scatter.x = _x;
    _c.scatter.y = _y;
    scatter_finalize_data(_c);
    
}

// 
void scatter_redraw(canvas_2d_t &_c, const axes_t &_axes)
{

    scatter_data_t &s = _c.scatter;
    normalized_params_t p(_c.params);

    std::vector<glm::vec2> marker_verts;
    size_t marker_vcount = figure_marker_vertices(p, marker_verts);

    std::vector<glm::vec3> V;
    V.reserve(s.x.size() * marker_vcount);

    for (size_t i = 0; i < s.x.size(); i++) {
        float x = _axes.eval_x(s.x[i]);
        float y = _axes.eval_y(s.y[i]);
        for (size_t j = 0; j < marker_vcount; j++) {
            V.push_back({ x + marker_verts[j].x,
                          y + marker_verts[j].y,
                          p.z_value_data });
        }
    }

    _c.vao_data.destroy();
    _c.vao_data.set_buffer_layout({{ VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }});
    _c.vao_data.create(V.data(), (uint32_t)V.size());

}

// 
void scatter_render(const canvas_2d_t &_c, shader_t &_shader)
{
    _shader.set_uniform_4fv("u_color", _c.params.data_color);
    _c.vao_data.bind();
    glDrawArrays(_c.gl_primitive, 0, _c.vao_data.m_vertex_count);
    _c.vao_data.unbind();
}


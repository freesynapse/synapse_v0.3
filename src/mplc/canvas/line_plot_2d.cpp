
#include "mplc/canvas/line_plot_2d.h"

#include "renderer/shader/shader.h"
#include "utils/log.h"


// 
void lineplot_finalize_data(canvas_2d_t &_c)
{
    lineplot_data_t &lp = _c.lineplot;

    SYN_ASSERT(lp.x.size() != lp.y.size(), "X and Y must have equal number of series.");

    _c.data_lim_x = { std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::lowest() };
    _c.data_lim_y = { std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::lowest() };

    lp.row_count = (int)lp.x.size();

    for (int i = 0; i < lp.row_count; i++) {
        SYN_ASSERT(lp.x[i].size() == lp.y[i].size(), "x and y must have equal length.");
        for (size_t j = 0; j < lp.x[i].size(); j++) {
            _c.data_lim_x[0] = std::min(_c.data_lim_x[0], lp.x[i][j]);
            _c.data_lim_x[1] = std::max(_c.data_lim_x[1], lp.x[i][j]);
            _c.data_lim_y[0] = std::min(_c.data_lim_y[0], lp.y[i][j]);
            _c.data_lim_y[1] = std::max(_c.data_lim_y[1], lp.y[i][j]);            
        }
    }

}

// 
void lineplot_data(canvas_2d_t &_c, const std::vector<float> &_y)
{
    std::vector<float> x(_y.size());
    for (size_t i = 0; i < _y.size(); i++) {
        x[i] = (float)i;
    }
    lineplot_data(_c, x, _y);
    
}

// 
void lineplot_data(canvas_2d_t &_c, const std::vector<float> &_x, const std::vector<float> &_y)
{
    _c.lineplot.x = { _x };
    _c.lineplot.y = { _y };
    lineplot_finalize_data(_c);
    
}
    

// 
void lineplot_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_y)
{
    _c.lineplot.x.clear();
    _c.lineplot.y = _y;

    for (size_t i = 0; i < _y.size(); i++) {
        std::vector<float> x(_y[i].size());
        for (size_t j = 0; j < _y[i].size(); j++) {
            x[j] = (float)j;
        }
        _c.lineplot.x.push_back(x);
    }
    lineplot_finalize_data(_c);
    
}

// 
void lineplot_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y)
{
    _c.lineplot.x = _x;
    _c.lineplot.y = _y;
    lineplot_finalize_data(_c);
    
}

// 
void lineplot_redraw(canvas_2d_t &_c, const axes_t &_axes)
{
    lineplot_data_t &lp = _c.lineplot;
    normalized_params_t p(_c.params);
    std::vector<glm::vec2> marker_verts;
    size_t marker_vcount = figure_marker_vertices(p, marker_verts);

    std::vector<glm::vec3> V;
    std::vector<glm::vec3> V_markers;

    for (int m = 0; m < lp.row_count; m++) {
        for (size_t n = 1; n < lp.x[m].size(); n++) {
            glm::vec3 v0 = { _axes.eval_x(lp.x[m][n-1]),
                             _axes.eval_y(lp.y[m][n-1]),
                             p.z_value_data };
            glm::vec3 v1 = { _axes.eval_x(lp.x[m][n]),
                             _axes.eval_y(lp.y[m][n]),
                             p.z_value_data };
            V.push_back(v0);
            V.push_back(v1);

            for (size_t i = 0; i < marker_vcount; i++) {
                V_markers.push_back({ v0.x + marker_verts[i].x,
                                      v0.y + marker_verts[i].y,
                                      p.z_value_data });
            }
        }
    }

    // line geometry
    _c.vao_data.destroy();
    _c.vao_data.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }
    });
    _c.vao_data.create(V.data(), (uint32_t)V.size());

    if (marker_vcount > 0 && !V_markers.empty()) {
        lp.marker_vcount = marker_vcount;
        lp.vao_markers.destroy();
        lp.vao_markers.set_buffer_layout({
            { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }            
        });
        lp.vao_markers.create(V_markers.data(), (uint32_t)V_markers.size());
        
        
    }
    
}

// 
void lineplot_render(const canvas_2d_t &_c, shader_t &_shader)
{
    const lineplot_data_t &lp = _c.lineplot;
    _shader.set_uniform_4fv("u_color", _c.params.data_color);

    if (_c.params.line_width_px != 1.0f) glLineWidth(_c.params.line_width_px);

    _c.vao_data.bind();
    glDrawArrays(_c.gl_primitive, 0, _c.vao_data.m_vertex_count);
    _c.vao_data.unbind();

    if (_c.params.line_width_px != 1.0f) glLineWidth(1.0f);

    if (lp.marker_vcount > 0) {
        lp.vao_markers.bind();
        glDrawArrays(GL_TRIANGLES, 0, lp.marker_vcount);
        lp.vao_markers.unbind();
    }

}

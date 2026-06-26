
#include <algorithm>
#include <math.h>

#include "mplc/canvas/histogram_2d.h"

#include "renderer/shader/shader.h"
#include "mplc/figure_utils.h"

// 
static void histogram_setup_bins(canvas_2d_t &_c, const glm::vec2 &_lim) 
{
    histogram_data_t &h = _c.histogram;
    h.bins.clear();

    if (h.bin_count < 0) {
        std::vector<float> sorted = h.data;
        std::sort(sorted.begin(), sorted.end());
        glm::vec2 q = iqr(sorted);
        float w = (2.0f * (q[1] - q[0])) / cbrtf((float)h.data.size());
        h.bin_count = (w > 0.0f) ? (int)floorf((_lim[1] - _lim[0])) / w : 1;
    }

    if (h.bin_count > 1) h.bins_dx = (_lim[1] - _lim[0]) / (h.bin_count - 1);
    else h.bins_dx = 1.0f;

    float x = _lim[0];
    for (int i = 0; i < h.bin_count - 1; i++) {
        h.bins[x] = 0;
        x += h.bins_dx;
    }
    
    h.bins[_lim[1]] = 0;    // last bin set manually to avoid rounding error
    
}

// 
void histogram_finalize_data(canvas_2d_t &_c)
{
    histogram_data_t &h = _c.histogram;

    glm::vec2 lim = { std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::lowest() };

    for (float v : h.data) {
        lim[0] = std::min(lim[0], v);
        lim[1] = std::max(lim[1], v);
    }

    histogram_setup_bins(_c, lim);

    for (float v : h.data) {
        if (v < lim[0] || v > lim[1]) continue;
        std::prev(h.bins.upper_bound(v))->second++;
    }

    _c.data_lim_x = { h.bins.begin()->first, h.bins.rbegin()->first };
    _c.data_lim_y = { 0.0f, std::numeric_limits<float>::lowest() };
    for (const auto &bin : h.bins) {
        _c.data_lim_y[1] = std::max(_c.data_lim_y[1], (float)bin.second);
    }
    
}

// 
void histogram_data(canvas_2d_t &_c, const std::vector<float> &_data)
{
    _c.histogram.data = _data;
    histogram_finalize_data(_c);
    
}

// 
void histogram_redraw(canvas_2d_t &_c, const axes_t &_axes)
{
    histogram_data_t &h = _c.histogram;
    normalized_params_t p(_c.params);

    std::vector<glm::vec3> V;
    V.reserve(h.bin_count * 4);
    std::vector<uint32_t> I;
    I.reserve(h.bin_count * 6);

    const float z = p.z_value_data;
    const float xoff = 0.5f * p.bar_spacing;

    uint32_t idx = 0;
    for (auto it = h.bins.begin(); it != h.bins.end(); it++) {
        float x0 = _axes.eval_x(it->first) + xoff;
        float x1 = _axes.eval_x(it->first + h.bins_dx) - xoff;
        float y0 = _axes.eval_y(0.0f);
        float y1 = _axes.eval_y((float)it->second);

        V.push_back({ x0, y0, z }); V.push_back({ x1, y0, z }); V.push_back({ x1, y1, z }); V.push_back({ x0, y1, z });
        I.push_back(idx + 0); I.push_back(idx + 1); I.push_back(idx + 2);
        I.push_back(idx + 2); I.push_back(idx + 3); I.push_back(idx + 0);
        idx += 4;
    }

    _c.vao_data.destroy();
    _c.vao_data.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }
    });
    _c.vao_data.create(V.data(), (uint32_t)V.size(), I.data(), (uint32_t)I.size());
    
}

// 
void histogram_render(const canvas_2d_t &_c, shader_t &_shader)
{
    _shader.set_uniform_4fv("u_color", _c.params.data_color);
    _c.vao_data.bind();
    glDrawElements(_c.gl_primitive, _c.vao_data.m_index_count, GL_UNSIGNED_INT, nullptr);
    _c.vao_data.unbind();
}


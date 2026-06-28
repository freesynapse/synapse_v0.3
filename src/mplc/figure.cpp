
#include "mplc/figure.h"

#include "mplc/canvas/canvas_2d.h"
#include "utils/log.h"
#include "renderer/font/font.h"

#include "c_api.h"

// 
static figure_type_t figure_type_from_canvas(canvas_type_t _type)
{
    switch (_type)
    {
        case canvas_type_t::SCATTER:   return figure_type_t::SCATTERPLOT;
        case canvas_type_t::LINEPLOT:  return figure_type_t::LINEPLOT;
        case canvas_type_t::HISTOGRAM: return figure_type_t::HISTOGRAM;
        default:                       return figure_type_t::NONE;
    }
}

// 
static canvas_2d_t *resolve_canvas(std::vector<std::unique_ptr<canvas_2d_t>> &_canvases,
                                   const std::string &_id)
{
    if (_id.empty()) {
        if (_canvases.size() == 1) {
            return _canvases[0].get();
        }
        SYN_WARNING("multiple canvases present; specify id");
        return nullptr;
    }

    for (auto &c : _canvases) {
        if (c->id == _id) return c.get();
    }

    SYN_WARNING("canvas '%s' not found.\n", _id.c_str());
    return nullptr;
    
}

// 
void figure_t::scatter(const std::vector<float> &_x,
                       const std::vector<float> &_y,
                       const std::string &_id,
                       scatter_params_t _params)
{
    canvas_2d_t c;
    c.type = canvas_type_t::SCATTER;
    c.id = canvas_resolve_id(m_canvases, _id);
    c.gl_primitive = GL_TRIANGLES;
    memcpy(&c.params, &m_params, sizeof(figure_params_t));
    c.params.set_from_scatter_params(_params);
    c.scatter.x = _x;
    c.scatter.y = _y;
    add_canvas(c);
    
}

// 
void figure_t::scatter(const std::vector<float> &_y,
                       const std::string &_id,
                       scatter_params_t _params)
{
    std::vector<float> x(_y.size());
    for (size_t i = 0; i < _y.size(); i++)
        x[i] = (float)i;
    scatter(x, _y, _id, _params);
    
}

// 
void figure_t::lineplot(const std::vector<std::vector<float>> &_x,
                        const std::vector<std::vector<float>> &_y,
                        const std::string &_id,
                        lineplot_params_t _params)
{
    canvas_2d_t c;
    c.type = canvas_type_t::LINEPLOT;
    c.id = canvas_resolve_id(m_canvases, _id);
    c.gl_primitive = GL_LINES;
    memcpy(&c.params, &m_params, sizeof(figure_params_t));
    c.params.set_from_lineplot_params(_params);
    c.lineplot.x = _x;
    c.lineplot.y = _y;
    c.lineplot.row_count = (int)_x.size();
    add_canvas(c);
    
}

// 
void figure_t::lineplot(const std::vector<float> &_x,
                        const std::vector<float> &_y,
                        const std::string &_id,
                        lineplot_params_t _params)
{
    std::vector<std::vector<float>> x = { _x };
    std::vector<std::vector<float>> y = { _y };
    lineplot(x, y, _id, _params);
    
}

// 
void figure_t::lineplot(const std::vector<float> &_y,
                        const std::string &_id,
                        lineplot_params_t _params)
{
    std::vector<float> x(_y.size());
    for (size_t i = 0; i < _y.size(); i++)
        x[i] = (float)i;
    lineplot(x, _y, _id, _params);
    
}

// 
void figure_t::histogram(const std::vector<float> &_data,
                         const std::string &_id,
                         histogram_params_t _params)
{
    canvas_2d_t c;
    c.type = canvas_type_t::HISTOGRAM;
    c.id = canvas_resolve_id(m_canvases, _id);
    c.gl_primitive = GL_TRIANGLES;
    memcpy(&c.params, &m_params, sizeof(figure_params_t));
    c.params.set_from_histogram_params(_params);
    c.histogram.data = _data;
    c.histogram.bin_count = _params.bin_count;
    add_canvas(c);
    
}

// 
void figure_t::data(const std::vector<float> &_y, const std::string &_id)
{
    canvas_2d_t *c = resolve_canvas(m_canvases, _id);
    if (!c) return;
    canvas_data(*c, _y);
    m_redraw_flags |= FIGURE_REDRAW_DATA;
    update_data_limits();

}

// 
void figure_t::data(const std::vector<float> &_x,
                    const std::vector<float> &_y,
                    const std::string &_id)
{
    canvas_2d_t *c = resolve_canvas(m_canvases, _id);
    if (!c) return;
    canvas_data(*c, _x, _y);
    m_redraw_flags |= FIGURE_REDRAW_DATA;
    update_data_limits();
    
}

// 
void figure_t::data(const std::vector<std::vector<float>> &_y, const std::string &_id)
{
    canvas_2d_t *c = resolve_canvas(m_canvases, _id);
    if (!c) return;
    canvas_data(*c, _y);
    m_redraw_flags |= FIGURE_REDRAW_DATA;
    update_data_limits();

}

// 
void figure_t::data(const std::vector<std::vector<float>> &_x,
                    const std::vector<std::vector<float>> &_y,
                    const std::string &_id)
{
    canvas_2d_t *c = resolve_canvas(m_canvases, _id);
    if (!c) return;
    canvas_data(*c, _x, _y);
    m_redraw_flags |= FIGURE_REDRAW_DATA;
    update_data_limits();
    
}

// 
void figure_t::render()
{
    // TODO : somewhere we are getting an invalid error code, before this
    // TODO : go through all glDraw* calls to isolate the error...
    // while (glGetError() != GL_NO_ERROR) {}
    if (!m_initialized) return;
    if (!m_redraw_flags) return;

    normalized_params_t p(m_params);
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_fbo_handle);
    if (!fbo) return;

    shader_t *shader = shader_lib.get_shader(m_geom_shader_handle);
    if (!shader) return;

    // bind fbo (and set viewport; true)
    fbo->bind(true);
    api.disable_depth_test();
    
    fbo->clear(p.figure_background);

    // First, rebuild all dirty geometry
    // 
    if (m_redraw_flags & FIGURE_REDRAW_AXES) redraw_axes(p);
    if (m_redraw_flags & FIGURE_REDRAW_TICKS) redraw_ticks(p);
    if (m_redraw_flags & FIGURE_REDRAW_DATA) {
        for (auto &c : m_canvases) 
            canvas_redraw(*c, m_axes);
    }
    if (m_redraw_flags & FIGURE_REDRAW_FILL) redraw_fill(p);

    
    // the actual rendering
    // 
    
    shader->enable();

    // 1. grid lines
    if ((m_aux_flags & FIGURE_AUX_GRIDLINES) && m_vao_grid.m_vertex_count > 0) {
        shader->set_uniform_4fv("u_color", { 0.3f, 0.3f, 0.3f, 1.0f });
        m_vao_grid.bind();
        glDrawArrays(GL_LINES, 0, m_vao_grid.m_vertex_count);
        m_vao_grid.unbind();
    }

    // 2. fill
    if ((m_aux_flags & (FIGURE_AUX_FILL_X | FIGURE_AUX_FILL_Y)) &&
        m_vao_fill.m_vertex_count > 0) {
        shader->set_uniform_4fv("u_color", p.fill_between_color);
        m_vao_fill.bind();
        glDrawArrays(GL_TRIANGLES, 0, m_vao_fill.m_vertex_count);
        m_vao_fill.unbind();
    }

    // 3. axes
    if (m_vao_axes.m_vertex_count > 0) {
        shader->set_uniform_4fv("u_color", p.axis_color);
        m_vao_axes.bind();
        glDrawArrays(GL_LINES, 0, (GLsizei)m_vao_axes.m_vertex_count);
        m_vao_axes.unbind();
    }

    // 4. ticks
    if (m_vao_ticks.m_vertex_count > 0) {
        shader->set_uniform_4fv("u_color", p.tick_color);
        m_vao_ticks.bind();
        glDrawArrays(GL_LINES, 0, m_vao_ticks.m_vertex_count);
        m_vao_ticks.unbind();
    }

    // 5. data
    for (auto &c : m_canvases) canvas_render(*c, *shader);

    // 
    shader->disable();

    // 6. tick labels uses the font shader
    if (m_redraw_flags & FIGURE_REDRAW_TICK_LABELS) redraw_tick_labels(p);
    
    // reset fbo and viewport
    fbo->unbind();
    api.enable_depth_test();
    glViewport(0, 0, root_window.get_width(), root_window.get_height());

    m_redraw_flags = 0;
    
}

// 
void figure_t::draw(window_handle_t &_window)
{
    if (!m_initialized) return;

    window_t *w = window_manager.get_window(_window);
    if (!w || !w->is_visible()) return;

    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_fbo_handle);
    if (!fbo) return;

    shader_t *shader = shader_lib.get_shader(renderer.m_ui_tex_quad_shader_handle);
    if (!shader) return;

    glm::vec2 p = w->get_content_position();
    glm::vec2 s = w->get_content_size();
    glm::vec2 pos;
    
    switch (m_placement) {
        case figure_placement_t::TOP_LEFT:  pos = p; break;
        case figure_placement_t::CENTER:    pos = p + (s - m_size_px) * 0.5f; break;
        case figure_placement_t::CUSTOM:    pos = p + m_placement_offset; break;
    }
    
    shader->enable();
    shader->set_matrix_4fv("u_projection", renderer.get_ui_projection_matrix());
    shader->set_uniform_2fv("u_position", pos);
    shader->set_uniform_2fv("u_size", m_size_px);
    shader->set_uniform_1f("u_depth", w->depth + window_manager.ddepth_layer_widget);

    fbo->bind_texture(0, 0);

    renderer.m_ui_tex_quad_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    renderer.m_ui_tex_quad_vao.unbind();

    shader->disable();
    
}

// 
canvas_2d_t *figure_t::find_canvas(const std::string &_id)
{
    for (auto &c : m_canvases) {
        if (c->id == _id)
            return c.get();
    }
    return nullptr;
    
}

// 
void figure_t::set_x_lim(const glm::vec2 &_lim)
{
    m_axes.set_x_lim(_lim, m_params.x_nice_scale);
    m_redraw_flags |= FIGURE_REDRAW_ALL;
}

// 
void figure_t::set_y_lim(const glm::vec2 &_lim)
{
    m_axes.set_y_lim(_lim, m_params.y_nice_scale);
    m_redraw_flags |= FIGURE_REDRAW_ALL;
}

// 
void figure_t::fill_x(const glm::vec2 &_lim)
{
    m_fill_lim_x    = _lim;
    m_aux_flags     |= FIGURE_AUX_FILL_X;
    m_redraw_flags  |= FIGURE_REDRAW_FILL;
}

// 
void figure_t::fill_y(const glm::vec2 &_lim)
{
    m_fill_lim_y    = _lim;
    m_aux_flags     |= FIGURE_AUX_FILL_Y;
    m_redraw_flags  |= FIGURE_REDRAW_FILL;
}

// 
void figure_t::disable_fill_x()
{
    m_fill_lim_x    = { 1.0f, -1.0f };
    m_aux_flags     &= ~FIGURE_AUX_FILL_X;
    m_redraw_flags  |= FIGURE_REDRAW_FILL;
}

// 
void figure_t::disable_fill_y()
{
    m_fill_lim_y    = { 1.0f, -1.0f };
    m_aux_flags     &= ~FIGURE_AUX_FILL_Y;
    m_redraw_flags  |= FIGURE_REDRAW_FILL;
}

// 
void figure_t::init(const glm::vec2 &_size_px, const std::string &_fbo_id)
{
    m_size_px = _size_px;
    m_params  = figure_params_t(_size_px);
    m_axes    = axes_t(m_params);

    // fbo
    m_fbo_handle = api.fbo_handler.create_framebuffer(
        color_format_t::RGBA16F,
        glm::ivec2((int)_size_px.x, (int)_size_px.y),
        1, 
        true, 
        _fbo_id);

    // fonts must be created while the fbo viewport is active so
    // ortho projection matches figure size, not root window size
    // 
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_fbo_handle);
    fbo->bind(true); // sets viewport

    m_font_tick_label = new font_t();
    m_font_tick_label->init("../assets/font/JetBrainsMono-Regular.ttf", m_params.tick_label_font_size_px);

    m_font_title = new font_t();
    m_font_title->init("../assets/font/JetBrainsMono-Regular.ttf", m_params.title_font_size_px);

    fbo->bind_default_framebuffer();

    // 
    m_geom_shader_handle = shader_lib.load_from_file("mplc_geom", "../assets/shaders/mplc/mplc_geom.glsl");
    
    // 
    m_initialized  = true;
    m_redraw_flags = FIGURE_REDRAW_ALL;
    
}

// 
void figure_t::destroy()
{
    if (!m_initialized) return;

    for (auto &c : m_canvases) {
        canvas_destroy(*c.get());
    }
    m_canvases.clear();

    m_font_tick_label->destroy(); delete m_font_tick_label; m_font_tick_label = nullptr;
    m_font_title->destroy();      delete m_font_title;      m_font_title      = nullptr;
  
    m_vao_axes.destroy();
    m_vao_ticks.destroy();
    m_vao_grid.destroy();
    m_vao_fill.destroy();

    m_fbo_handle = { 0 };

    m_initialized = false;
    
}

// 
void figure_t::add_canvas(canvas_2d_t &_canvas)
{
    // histogram must be the only canvas
    if (_canvas.type == canvas_type_t::HISTOGRAM && !m_canvases.empty()) {
        SYN_WARNING("histogram must be the only canvas -- canvas '%s' not added.\n", _canvas.id.c_str());
        return;
    }

    for (const auto &c : m_canvases) {
        if (c->type == canvas_type_t::HISTOGRAM) {
            SYN_WARNING("histogram already present -- canvas '%s' not added.\n", _canvas.id.c_str());
            return;
        }
    }

    _canvas.params.figure_type = figure_type_from_canvas(_canvas.type);
    m_canvases.push_back(std::make_unique<canvas_2d_t>(std::move(_canvas)));
    canvas_finalize_data(*m_canvases.back());
    update_data_limits();
    
}

// 
void figure_t::update_data_limits()
{
    m_data_lim_x = { std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::lowest() };
    m_data_lim_y = { std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::lowest() };

    bool x_nice = true;
    bool y_nice = true;
    float hist_x_add = 0.0f;

    for (const auto &c : m_canvases)
    {
        m_data_lim_x[0] = std::min(m_data_lim_x[0], c->data_lim_x[0]);
        m_data_lim_x[1] = std::max(m_data_lim_x[1], c->data_lim_x[1]);
        m_data_lim_y[0] = std::min(m_data_lim_y[0], c->data_lim_y[0]);
        m_data_lim_y[1] = std::max(m_data_lim_y[1], c->data_lim_y[1]);

        x_nice &= c->params.x_nice_scale;
        y_nice &= c->params.y_nice_scale;

        if (c->type == canvas_type_t::HISTOGRAM)
            hist_x_add = c->histogram.bins_dx;
    }

    m_params.x_nice_scale = x_nice;
    m_params.y_nice_scale = y_nice;

    if (m_data_lim_x != m_data_lim_x_prev)
    {
        m_data_lim_x_prev = m_data_lim_x;
        nice_scale_t xs({ m_data_lim_x[0], m_data_lim_x[1] + hist_x_add }, x_nice);
        m_axes.set_x_lim({ xs.lower_bound, xs.upper_bound }, x_nice);
    }

    if (m_data_lim_y != m_data_lim_y_prev)
    {
        m_data_lim_y_prev = m_data_lim_y;
        nice_scale_t ys(m_data_lim_y, y_nice);
        m_axes.set_y_lim({ ys.lower_bound, ys.upper_bound }, y_nice);
    }

    // 
    
    m_redraw_flags = FIGURE_REDRAW_ALL;
    
}

// 
void figure_t::redraw_axes(const normalized_params_t &_p)
{
    std::vector<glm::vec3> V;
    const float z = _p.z_value_aux;

    // x
    if (_p.render_x_axis) {
        V.push_back({ _p.x_axis_lim[0] - _p.axes_neg_protrusion.x, _p.canvas_origin.y, z });
        V.push_back({ _p.x_axis_lim[1], _p.canvas_origin.y, z });
    }

    // y
    if (_p.render_y_axis) {
        V.push_back({ _p.canvas_origin.x, _p.y_axis_lim[0] - _p.axes_neg_protrusion.y, z });
        V.push_back({ _p.canvas_origin.x, _p.y_axis_lim[1], z });
    }

    // grid lines here since they share tick positions
    if (m_aux_flags & FIGURE_AUX_GRIDLINES) {
        std::vector<glm::vec3> G;
        const float gz = _p.z_value_aux - 0.01f;

        // x gridlines
        nice_scale_t &xs = m_axes.x_ticks();
        if (xs.set) {
            float t = xs.lower_bound;
            while (t <= xs.upper_bound + 1e-6f) {
                float x = m_axes.eval_x(t);
                G.push_back({ x, _p.y_axis_lim[0], gz });
                G.push_back({ x, _p.y_axis_lim[1], gz });
                t += xs.tick_spacing;
            }
        }

        // y gridlines
        nice_scale_t &ys = m_axes.y_ticks();
        if (ys.set) {
            float t = ys.lower_bound;
            while (t <= ys.upper_bound + 1e-6f) {
                float y = m_axes.eval_y(t);
                G.push_back({ _p.x_axis_lim[0], y, gz });
                G.push_back({ _p.x_axis_lim[1], y, gz });
                t += ys.tick_spacing;
            }
        }

        m_vao_grid.destroy();
        m_vao_grid.set_buffer_layout({{ VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }});
        if (!G.empty()) m_vao_grid.create(G.data(), (uint32_t)G.size());
    }

    m_vao_axes.destroy();
    m_vao_axes.set_buffer_layout({{ VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }});
    if (!V.empty()) m_vao_axes.create(V.data(), (uint32_t)V.size());

}

// 
void figure_t::redraw_ticks(const normalized_params_t &_p)
{
    std::vector<glm::vec3> V;
    const float z = _p.z_value_aux;

    m_tick_label_pos_x.clear();
    m_tick_label_pos_y.clear();

    // x
    if (_p.render_x_ticks) {
        nice_scale_t &xs = m_axes.x_ticks();
        if (xs.set) {
            float t = xs.lower_bound;
            while (t <= xs.upper_bound + 1e-6f)
            {
                float x = m_axes.eval_x(t);
                V.push_back({ x, _p.canvas_origin.y, z });
                V.push_back({ x, _p.canvas_origin.y - _p.tick_length.y, z });
                m_tick_label_pos_x.push_back({ x, _p.canvas_origin.y - _p.tick_length.y - _p.tick_labels_offset.y });

                t += xs.tick_spacing;
            }
        }
    }

    // y
    if (_p.render_y_ticks)
    {
        nice_scale_t &ys = m_axes.y_ticks();
        if (ys.set)
        {
            float t = ys.lower_bound;
            while (t <= ys.upper_bound + 1e-6f)
            {
                float y = m_axes.eval_y(t);
                V.push_back({ _p.canvas_origin.x, y, z });
                V.push_back({ _p.canvas_origin.x - _p.tick_length.x, y, z });
                m_tick_label_pos_y.push_back({ _p.canvas_origin.x - _p.tick_length.x - _p.tick_labels_offset.x, y });
                t += ys.tick_spacing;
            }
        }
    }

    m_vao_ticks.destroy();
    m_vao_ticks.set_buffer_layout({{ VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }});
    if (!V.empty()) m_vao_ticks.create(V.data(), (uint32_t)V.size());
    
}

// 
void figure_t::format_tick_labels(nice_scale_t &_ticks, const std::vector<glm::vec2> &_positions)
{
    _ticks.tick_labels.labels.clear();
    _ticks.tick_labels.count           = 0;
    _ticks.tick_labels.max_label_width = 0.0f;
    _ticks.tick_labels.min_label_width = std::numeric_limits<float>::max();

    // number of deciamls determined from tick_spacing
    int decimals = 0;
    float spacing = _ticks.tick_spacing;
    while (spacing < 1.0f && decimals < 6) {
        spacing *= 10.0f;
        decimals++;
    }

    char buf[64];
    float t = _ticks.lower_bound;
    for (size_t i = 0; i < _positions.size(); i++) {
        snprintf(buf, sizeof(buf), "%.*f", decimals, t);
        _ticks.tick_labels.labels.push_back(buf);
        t += _ticks.tick_spacing;
    }

    _ticks.tick_labels.count = _ticks.tick_labels.labels.size();
}

// 
void figure_t::redraw_tick_labels(const normalized_params_t &_p)
{
    if (!_p.render_ticklabels) return;
    if (!m_font_tick_label) return;

    format_tick_labels(m_axes.x_ticks(), m_tick_label_pos_x);
    format_tick_labels(m_axes.y_ticks(), m_tick_label_pos_y);

    // convert normalized [0..1] positions to pixel space for font rendering
    nice_scale_t &xs = m_axes.x_ticks();
    nice_scale_t &ys = m_axes.y_ticks();

    for (size_t i = 0; i < m_tick_label_pos_x.size() && i < xs.tick_labels.count; i++) {
        float px = m_tick_label_pos_x[i].x * m_size_px.x;
        float py = (1.0f - m_tick_label_pos_x[i].y) * m_size_px.y;
        m_font_tick_label->render_text_centered(px, py, xs.tick_labels.labels[i].c_str());
    }

    for (size_t i = 0; i < m_tick_label_pos_y.size() && i < ys.tick_labels.count; i++) {
        float px = m_tick_label_pos_y[i].x * m_size_px.x;
        float py = (1.0f - m_tick_label_pos_y[i].y) * m_size_px.y;
        m_font_tick_label->render_text_right(px, py, ys.tick_labels.labels[i].c_str());
    }

    glm::mat4 proj = glm::ortho(0.0f, m_size_px.x, m_size_px.y, 0.0f, -100.0f, 100.0f);
    m_font_tick_label->end_render_block_with_proj(proj);

    // title
    if (_p.render_title && !m_title.empty() && m_font_title) {
        m_font_title->render_text_centered(m_size_px.x * 0.5f, (1.0f - _p.y_axis_lim[1]) * m_size_px.y - 10.0f, m_title.c_str());
        m_font_title->end_render_block_with_proj(proj);
    }

}

// 
void figure_t::redraw_fill(const normalized_params_t &_p)
{
    std::vector<glm::vec3> V;
    const float z = _p.z_value_aux - 0.005f;

    // x
    if (m_aux_flags & FIGURE_AUX_FILL_X) {
        float x0 = m_axes.eval_x(m_fill_lim_x[0]);
        float x1 = m_axes.eval_x(m_fill_lim_x[1]);
        float y0 = _p.y_axis_lim[0];
        float y1 = _p.y_axis_lim[1];
        V.push_back({ x0, y0, z }); V.push_back({ x1, y0, z }); V.push_back({ x1, y1, z });
        V.push_back({ x1, y1, z }); V.push_back({ x0, y1, z }); V.push_back({ x0, y0, z });
    }

    // y
    if (m_aux_flags & FIGURE_AUX_FILL_Y) {
        float x0 = _p.x_axis_lim[0];
        float x1 = _p.x_axis_lim[1];
        float y0 = m_axes.eval_y(m_fill_lim_y[0]);
        float y1 = m_axes.eval_y(m_fill_lim_y[1]);
        V.push_back({ x0, y0, z }); V.push_back({ x1, y0, z }); V.push_back({ x1, y1, z });
        V.push_back({ x1, y1, z }); V.push_back({ x0, y1, z }); V.push_back({ x0, y0, z });
    }

    m_vao_fill.destroy();
    m_vao_fill.set_buffer_layout({{ VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }});
    if (!V.empty()) m_vao_fill.create(V.data(), (uint32_t)V.size());
    
}

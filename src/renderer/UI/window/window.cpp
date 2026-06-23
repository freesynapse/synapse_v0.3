
#include <string.h>

#include "renderer/UI/window/window.h"
#include "renderer/entity/entity_library.h"
#include "utils/log.h"

#include "c_api.h"


// 
void window_t::init()
{
        // default flags
    m_is_visible = true;
    m_is_active = true;

}

// 
void window_t::destroy()
{
    destroy_framebuffer();
    
}

// 
void window_t::create_framebuffer()
{
    if (m_has_framebuffer) {
        destroy_framebuffer();
    }

    glm::vec2 content_size = get_content_size();
    if (content_size.x <= 0 || content_size.y <= 0) {
        SYN_WARNING("invalid framebuffer size for window '%s'.\n", name.c_str());
        return;
    }
    std::string id = name + "_framebuffer";
    m_framebuffer_handle = api.fbo_handler.create_framebuffer(
        color_format_t::RGBA16F, 
        glm::ivec2(content_size.x, content_size.y), 
        1, true, id);

    cam.set_aspect_ratio(content_size.x / content_size.y);
    cam.update_projection_matrix();
    
    m_has_framebuffer = (m_framebuffer_handle.id != 0);

    SYN_INFO("created framebuffer for window '%s' (%dx%d).\n", 
        name.c_str(), (int)content_size.x, (int)content_size.y);
}

// 
void window_t::resize_framebuffer()
{
    if (m_is_tab_container) {
        glm::vec2 content_pos = get_content_position();
        glm::vec2 content_size = get_content_size();
        for (uint32_t i = 0; i < m_tab_count; i++) {
            // if this has a framebuffer, resize it in-place to avoid recursion
            if (m_tab_children[i].id == this_handle.id && m_has_framebuffer) {
                framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_framebuffer_handle);
                if (fbo) {
                    fbo->resize(glm::ivec2(content_size.x, content_size.y));
                    cam.set_aspect_ratio(content_size.x / content_size.y);
                    cam.update_projection_matrix();
                }
                continue;
            }
            window_t *child = window_manager.get_window(m_tab_children[i]);
            if (child && child->has_frambuffer()) {
                child->size = content_size;
                child->position = content_pos;
                child->resize_framebuffer();
            }
        }
        return;
    }

    if (!m_has_framebuffer) return;

    glm::vec2 content_size = get_content_size();
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_framebuffer_handle);

    if (fbo) {
        fbo->resize(glm::vec2((int)content_size.x, (int)content_size.y));
        cam.set_aspect_ratio(content_size.x / content_size.y);
        cam.update_projection_matrix();

        api.set_scene_viewport(content_size);
    }
}

// 
void window_t::destroy_framebuffer()
{
    if (m_has_framebuffer && m_framebuffer_handle.id == 0) {
        // TODO : implement this?
        // api.fbo_handler.release_framebuffer(m_framebuffer_handle);
    }
    m_framebuffer_handle = { 0 };
    m_has_framebuffer = false;
}

// 
void window_t::on_resize()
{
    // notify widgets
    // glm::vec2 content_size = get_content_size();
    // for (uint32_t i = 0; i < m_widget_count; i++) {
    //     if (m_widgets[i].on_resize) {
    //         m_widgets[i].on_resize(&m_widgets[i], content_size);
    //     }
    // }

    // reize framebuffer if present
    if (m_has_framebuffer || m_is_tab_container) {
        resize_framebuffer();
    }
    
}

// 
void window_t::draw()
{
    if (m_is_tab_child) return;
    
    glm::vec4 tb_color = m_is_focused ? title_bar_color_focused : title_bar_color;

    // title or tab bar
    if (m_is_tab_container) {
        draw_tab_bar();
        return;
    } else {
        renderer_2d.batch.add_quad(position, { size.x, title_bar_height }, tb_color, depth);
    }
    // content area
    renderer_2d.batch.add_quad({ position.x, position.y + title_bar_height }, 
                                { size.x, size.y - title_bar_height }, 
                                bg_color, depth);
    // lines
    glm::vec2 p = position;
    glm::vec2 s = size;
    float tb_h = title_bar_height;
    ui_render_vertex_t line_vertices[] = {
        ui_render_vertex_t({ p.x,       p.y + tb_h }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x + s.x, p.y + tb_h }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x + s.x, p.y + s.y  }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x,       p.y + s.y  }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x,       p.y        }, outline_color, depth + 0.03f),
    };
    renderer_2d.batch.add_line_strip(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));
    
    float tx = position.x + 10.0f;
    float ty = position.y + title_bar_height - font.get_font_height() * 0.5f;
    float font_depth = depth + window_manager.ddepth_layer_text;//depth + window_manager.ddepth_layer_text;//(depth + window_manager.ddepth_layer_text) / window_manager.m_zfar;
    font.set_color(fg_color);
    font.set_depth(font_depth);
    font.render_text(tx, ty, "%s", name.c_str());

    // render title bar close button
    float btn_size = 10.0f;
    float margin = 8.0f;
    glm::vec2 btn_p = { position.x + size.x - margin - btn_size, position.y + (title_bar_height - btn_size) * 0.5f };
    glm::vec2 btn_s = { btn_size, btn_size };
    glm::vec2 mpos = input.mouse_position;
    bool hovered = (mpos.x >= btn_p.x && mpos.x <= btn_p.x + btn_s.x &&
                    mpos.y >= btn_p.y && mpos.y <= btn_p.y + btn_s.y);
    glm::vec4 btn_color = hovered ? glm::vec4(0.8f, 0.2f, 0.2f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    renderer_2d.batch.add_quad(btn_p, btn_s, btn_color, depth + 0.01f);
    glm::vec4 ol_c = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    ui_render_vertex_t outline[] = {
        ui_render_vertex_t({ btn_p.x,            btn_p.y            }, ol_c, depth + 0.02f),
        ui_render_vertex_t({ btn_p.x + btn_s.x,  btn_p.y            }, ol_c, depth + 0.02f),
        ui_render_vertex_t({ btn_p.x + btn_s.x,  btn_p.y + btn_s.y  }, ol_c, depth + 0.02f),
        ui_render_vertex_t({ btn_p.x,            btn_p.y + btn_s.y  }, ol_c, depth + 0.02f),
        ui_render_vertex_t({ btn_p.x,            btn_p.y            }, ol_c, depth + 0.02f),
    };
    renderer_2d.batch.add_line_strip(outline, 5);

    m_close_btn_pos  = btn_p;
    m_close_btn_size = btn_s;
}

// 
void window_t::draw_tab_bar()
{
    float tab_y = position.y;
    float tab_h = title_bar_height;
    float tab_x = position.x;
    float tab_w = size.x / (float)m_tab_count;

    for (uint32_t i = 0; i < m_tab_count; i++) {
        window_t *tab = window_manager.get_window(m_tab_children[i]);
        if (!tab) continue;

        bool is_active = (i == m_active_tab);

        glm::vec4 tab_color = is_active ? title_bar_color_focused : title_bar_color;
        renderer_2d.batch.add_quad({ tab_x, tab_y }, { tab_w, tab_h }, tab_color, depth);

        // tab label, centered
        float text_x = tab_x + tab_w * 0.5f - font.get_string_width("%s", tab->name.c_str()) * 0.5f;
        float text_y = tab_y + tab_h - font.get_font_height() * 0.5f;
        float font_depth = (depth + window_manager.ddepth_layer_text) / window_manager.m_zfar;
        font.set_color(fg_color);
        font.set_depth(font_depth);
        font.render_text(text_x, text_y, "%s", tab->name.c_str());

        // divider between tabs (skipping the last tab)
        if (i < m_tab_count - 1) {
            float div_x = tab_x + tab_w;
            ui_render_vertex_t div[] = {
                ui_render_vertex_t({ div_x, tab_y }, outline_color, depth + 0.03f),
                ui_render_vertex_t({ div_x, tab_y + tab_h }, outline_color, depth + 0.03f)
            };
            renderer_2d.batch.add_line_strip(div, 2);
        }

        tab_x += tab_w;
    }

    // lines
    glm::vec2 p = position;
    glm::vec2 s = size;
    float tb_h = title_bar_height;
    ui_render_vertex_t line_vertices[] = {
        ui_render_vertex_t({ p.x,       p.y + tb_h }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x + s.x, p.y + tb_h }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x + s.x, p.y + s.y  }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x,       p.y + s.y  }, outline_color, depth + 0.03f),
        ui_render_vertex_t({ p.x,       p.y        }, outline_color, depth + 0.03f),
    };
    renderer_2d.batch.add_line_strip(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));

    // content area background
    renderer_2d.batch.add_quad({ p.x, p.y + tb_h }, { s.x, s.y - tb_h }, bg_color, depth);

}

// 
bool window_t::is_point_in_window(const glm::vec2 &_p)
{
    return (_p.x >= position.x && _p.x <= position.x + size.x &&
            _p.y >= position.y && _p.y <= position.y + size.y);
}

// 
bool window_t::is_point_in_title_bar(const glm::vec2 &_p)
{
    return (_p.x >= position.x && _p.x <= position.x + size.x &&
            _p.y >= position.y && _p.y <= position.y + title_bar_height);
}

// 
resize_handle_t window_t::get_resize_handle_at_pos(const glm::vec2 &_pos)
{
    float border = m_resize_border_width;

    bool on_left   = (_pos.x >= position.x && _pos.x <= position.x + border);
    bool on_right  = (_pos.x >= position.x + size.x - border && _pos.x <= position.x + size.x);
    bool on_top    = (_pos.y >= position.y && _pos.y <= position.y + border);
    bool on_bottom = (_pos.y >= position.y + size.y - border && _pos.y <= position.y + size.y);

    if (on_top && on_left)      return resize_handle_t::TOP_LEFT;
    if (on_top && on_right)     return resize_handle_t::TOP_RIGHT;
    if (on_bottom && on_left)   return resize_handle_t::BOTTOM_LEFT;
    if (on_bottom && on_right)  return resize_handle_t::BOTTOM_RIGHT;

    if (on_top)     return resize_handle_t::TOP;
    if (on_bottom)  return resize_handle_t::BOTTOM;
    if (on_left)    return resize_handle_t::LEFT;
    if (on_right)   return resize_handle_t::RIGHT;

    return resize_handle_t::NONE;
    
}

// 
void window_t::im_begin(float _padding)
{
    im_cursor_y = im_padding;
    
}

// 
widget_state_t *window_t::im_get_or_create_state(uint32_t _id)
{
    // 1. search
    for (uint32_t i = 0; i < im_widget_state_count; i++) {
        if (im_widget_states[i].id == _id) return &im_widget_states[i];
    }

    // 2. else, create
    if (im_widget_state_count >= SYN_IM_MAX_WIDGET_STATES) {
        SYN_WARNING("widget state pool full in window '%s'.\n", name.c_str());
        return nullptr;
    }

    widget_state_t &s = im_widget_states[im_widget_state_count++];
    s = widget_state_t{};
    s.id = _id;
    return &s;
    
}

// 
bool window_t::im_is_hovered(const glm::vec2 &_pos, const glm::vec2 &_size)
{
    glm::vec2 mpos = input.mouse_position;
    window_handle_t top = window_manager.get_window_at_pos(mpos);
    if (top.id != this_handle.id) return false;
    
    return (mpos.x >= _pos.x && mpos.x <= _pos.x + _size.x &&
            mpos.y >= _pos.y && mpos.y <= _pos.y + _size.y);
}

// 
void window_t::im_begin_row(const std::vector<float> &_ratios)
{
    im_in_row      = true;
    im_col_index   = 0;
    im_row_saved_y = im_cursor_y;
    im_row_max_h   = 0.0f;

    glm::vec2 content_pos  = get_content_position();
    glm::vec2 content_size = get_content_size();
    float available_w = content_size.x - im_padding * 2.0f;

    float total = 0.0f;
    for (float r : _ratios) total += r;

    im_col_x.clear();
    im_col_w.clear();
    float cursor_x = content_pos.x + im_padding;
    for (float r : _ratios) {
        float w = (r / total) * available_w;
        im_col_x.push_back(cursor_x);
        im_col_w.push_back(w - im_padding);
        cursor_x += w;
    }
}

// 
void window_t::im_end_row()
{
    im_in_row    = false;
    im_col_index = 0;
    im_cursor_y  = im_row_saved_y + im_row_max_h + im_padding;
    im_col_x.clear();
    im_col_w.clear();    
}

// 
void window_t::im_clear_states()
{
    memset(im_widget_states, 0, sizeof(widget_state_t) * SYN_IM_MAX_WIDGET_STATES);
    im_widget_state_count = 0;
    im_active_state_id = 0;
    
}

// 
window_handle_t window_t::get_active_tab_child_handle()
{
    if (!m_is_tab_container || m_tab_count == 0) return { 0 };
    return m_tab_children[m_active_tab];
    
}

int window_t::get_tab_index_at_pos(const glm::vec2 &_pos)
{
    if (!m_is_tab_container) return -1;
    if (_pos.x < position.x || _pos.x > position.x + size.x ||
        _pos.y < position.y || _pos.y > position.y + title_bar_height) return -1;

    float tab_w = size.x / (float)m_tab_count;
    int idx = (int)((_pos.x - position.x) / tab_w);
    return glm::clamp(idx, 0, (int)m_tab_count - 1);
    
}


//---------------------------------------------------------------------------------------
// immediate mode helper functions 
// 
void im_set_widget_params(window_t *_window, widget_params_t *_params)
{
    _params->content_pos  = _window->get_content_position();
    _params->content_size = _window->get_content_size();
    _params->pad          = _window->im_padding;
    _params->row_h        = _window->im_default_row_height;

    if (_window->im_in_row && _window->im_col_index < (int)_window->im_col_w.size()) {
        _params->col_x = _window->im_col_x[_window->im_col_index];
        _params->col_w = _window->im_col_w[_window->im_col_index];
        _params->in_row = true;
    } else {
        _params->col_x = _params->content_pos.x + _params->pad;
        _params->col_w = _params->content_size.x - _params->pad * 2.0f;
        _params->in_row = false;
    }

    _params->p = { _params->col_x, _params->content_pos.y + _window->im_cursor_y };
    _params->s = { _params->col_w, _params->row_h };
}

// 
void im_update_cursor_y(window_t *_window, widget_params_t *_params)
{
    if (_window->im_in_row) {
        _window->im_row_max_h = std::max(_window->im_row_max_h, _params->s.y);
        _window->im_col_index++;
    } else {
        _window->im_cursor_y += _params->s.y + _params->pad;
    }
}

// 
bool im_needs_update(window_t *_window)
{
    bool needs_update = false;
    for (uint32_t i = 0; i < _window->im_widget_state_count; i++) {
        if (_window->im_widget_states[i].is_dirty) {
            needs_update = true;
            _window->im_widget_states[i].is_dirty = false;
        }
    }

    return needs_update;    
}

// 
uint32_t im_widget_hash(const char *_str, window_t *_window)
{
    uint32_t hash = 2166136261u;
    while (*_str) {
        hash ^= (uint8_t)*_str++;
        hash *= 16777619u;
    }
    uint32_t cursor_bits = (uint32_t)(_window->im_cursor_y * 100.0f);
    hash ^= cursor_bits;
    hash *= 16777619u;
    return hash ? hash : 1u;
    
}

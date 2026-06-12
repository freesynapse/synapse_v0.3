
#include "renderer/UI/window/window.h"
#include "utils/log.h"

#include "c_api.h"


// 
void window_t::init()
{
    
    widget_t close_btn;
    close_btn.type          = widget_type_t::BUTTON;
    close_btn.anchor        = widget_anchor_t::TOP_RIGHT;
    close_btn.position      = glm::vec2(8.0f, 8.0f);
    close_btn.size          = glm::vec2(10.0f, 10.0f);
    close_btn.color         = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    close_btn.hover_color   = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    close_btn.outline_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    close_btn.in_title_bar  = true;
    close_btn.on_click = [this]() {
        event_t e;
        e.type = event_type_t::UI_WINDOW_CLOSE;
        e.as.ui_window_close.handle = this->this_handle;
        events.dispatch_event(e);
    };

    m_widget_count = 0;
    add_widget(close_btn);
    
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
    glm::vec2 content_size = get_content_size();
    for (uint32_t i = 0; i < m_widget_count; i++) {
        if (m_widgets[i].on_resize) {
            m_widgets[i].on_resize(&m_widgets[i], content_size);
        }
    }

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
    float font_depth = (depth + window_manager.m_ddepth_layer_text) / window_manager.m_zfar;
    font.set_color(fg_color);
    font.set_depth(font_depth);
    font.render_text(tx, ty, "%s", name.c_str());

    draw_widgets();

}

// 
void window_t::draw_widgets()
{
    glm::vec2 content_pos  = glm::vec2(position.x, position.y + title_bar_height);
    glm::vec2 content_size = glm::vec2(size.x, size.y - title_bar_height);
    
    for (uint32_t i = 0; i < m_widget_count; i++) {
        // 
        widget_t *w = &m_widgets[i];
        if (!w->is_visible) continue;

        glm::vec2 p = w->in_title_bar ? w->get_absolute_position(position, size) : w->get_absolute_position(content_pos, content_size);
        glm::vec4 fg_c = w->is_hovered ? w->hover_color : w->color;
        glm::vec4 ol_c = w->outline_color;
        float depth_offset = (depth + 0.01f);

        switch (w->type) {
            case widget_type_t::BUTTON: {
                renderer_2d.batch.add_quad(p, w->size, fg_c, depth_offset);

                glm::vec2 s = w->size;
                ui_render_vertex_t line_vertices[] = {
                    ui_render_vertex_t({ p.x,       p.y       }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x + s.x, p.y       }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x + s.x, p.y + s.y }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x,       p.y + s.y }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x,       p.y       }, ol_c, depth_offset + 0.01f),
                };
                renderer_2d.batch.add_line_strip(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));

                if (!w->text.empty()) {
                    float x = p.x + 0.5f * w->size.x - 0.5f * font.get_string_width("%s", w->text.c_str());
                    float y = p.y + w->size.y - 0.5f * font.get_font_height();
                    font.set_depth(depth_offset + 0.02f);
                    font.render_text(x, y, "%s", w->text.c_str());
                }
                
                break;
            }

            case widget_type_t::LABEL: {
                break;
            }

            case widget_type_t::TEXT_AREA: {
                if (!w->get_lines) break;

                float prev_depth = font.get_current_depth();
                float font_depth = (depth + window_manager.m_ddepth_layer_text) / window_manager.m_zfar;
                font.set_depth(font_depth);
                glm::vec4 prev_color = font.get_color();

                // 
                float line_h = font.get_font_height();
                uint32_t max_lines = (uint32_t)(w->size.y / line_h);

                // get the text
                text_area_line_t lines[256];
                uint32_t count = w->get_lines(lines, 256u);

                // scrolling
                w->scroll_max_lines = (count > max_lines) ? count - max_lines : 0;
                uint32_t start = w->scroll_max_lines - (uint32_t)w->scroll_offset;
                uint32_t display_count = (count > start) ? std::min(count - start, max_lines) : 0;

                // render text
                for (uint32_t j = 0; j < display_count; j++) {
                    font.set_color(lines[start + j].color);
                    font.render_text_clipped(p.x + 4.0f, p.y + j * line_h + line_h, w->size.x - 8.0f, "%s", lines[start + j].text);
                }
                font.set_depth(prev_depth);
                font.set_color(prev_color);
                
                break;
            }
            
            default:
                break;
        }
    }
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
        float font_depth = (depth + window_manager.m_ddepth_layer_text) / window_manager.m_zfar;
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

    draw_widgets();
    
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
void window_t::add_widget(const widget_t &_widget)
{
    if (m_widget_count >= SYN_WINDOW_MAX_WIDGET_COUNT) {
        SYN_ERROR("window '%s' widget pool full.\n", name.c_str());
        return;
    }

    m_widgets[m_widget_count++] = _widget;
}

// 
widget_t *window_t::get_widget(uint32_t _index)
{
    if (_index >= m_widget_count) return nullptr;
    return &m_widgets[_index];
}

// 
widget_t *window_t::get_widget_at_pos(const glm::vec2 &_pos)
{
    for (int i = m_widget_count - 1; i >= 0; i--) {
        widget_t *w = &m_widgets[i];
        if (w->is_visible && w->is_enabled && w->contains_point(position, size, _pos, title_bar_height)) {
            return w;
        }
    }
    
    return nullptr;
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

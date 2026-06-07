
#include "renderer/UI/window/window.h"
#include "utils/log.h"

#include "c_api.h"


// 
void window_t::init()
{
    widget_t close_btn;
    close_btn.type          = widget_type_t::BUTTON;
    close_btn.position      = glm::vec2(size.x - 25.0f, 3.0f);
    close_btn.size          = glm::vec2(20.0f, 20.0f);
    close_btn.color         = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    close_btn.hover_color   = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    close_btn.outline_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    close_btn.on_click = [this]() {
        event_t e;
        e.type = event_type_t::UI_WINDOW_CLOSE;
        e.as.ui_window_close.handle = this->this_handle;
        events.dispatch_event(e);
    };
    add_widget(close_btn);
    
    // default flags
    m_is_visible = true;
    m_is_active = true;

}

// 
void window_t::destroy()
{

}

// 
void window_t::draw()
{
    glm::vec4 tb_color = m_is_focused ? title_bar_color_focused : title_bar_color;
    ui_batch_renderer.add_quad(position, { size.x, title_bar_height }, tb_color, depth);

    ui_batch_renderer.add_quad({ position.x, position.y + title_bar_height }, 
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
    ui_batch_renderer.add_line_strip(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));
    
    float tx = position.x + 10.0f;
    float ty = position.y + (title_bar_height - font.get_font_height() * 0.5f);
    font.set_color(fg_color);
    float font_depth = (depth + window_manager.m_ddepth_layer_text) / window_manager.m_zfar;
    font.set_depth(font_depth);
    font.render_text(tx, ty, "%s", name.c_str());

    draw_widgets();

}

// 
void window_t::draw_widgets()
{
    for (uint32_t i = 0; i < m_widget_count; i++) {
        // 
        widget_t *w = &m_widgets[i];
        if (!w->is_visible) return;

        glm::vec2 p = position + w->position;
        glm::vec2 s = w->size;
        glm::vec4 fg_c = w->is_hovered ? w->hover_color : w->color;
        glm::vec4 ol_c = w->outline_color;
        float depth_offset = (depth + 0.01f);

        switch (w->type) {
            case widget_type_t::BUTTON: {
                ui_batch_renderer.add_quad(p, w->size, fg_c, depth_offset);

                ui_render_vertex_t line_vertices[] = {
                    ui_render_vertex_t({ p.x,       p.y       }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x + s.x, p.y       }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x + s.x, p.y + s.y }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x,       p.y + s.y }, ol_c, depth_offset + 0.01f),
                    ui_render_vertex_t({ p.x,       p.y       }, ol_c, depth_offset + 0.01f),
                };
                ui_batch_renderer.add_line_strip(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));

                if (!w->text.empty()) {
                    float x = p.x + 0.5f * w->size.x - 0.5f * font.get_string_width("%s", w->text.c_str());
                    float y = p.y + w->size.y - 0.5f * font.get_font_height();
                    font.set_depth(depth_offset + 0.02f);
                    font.render_text(x, y, "%s", w->text.c_str());
                }
                
                break;
            }

            case widget_type_t::LABEL:
                break;

            default:
                break;
        }
    }
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
        if (w->is_visible && w->is_enabled && w->contains_point(position, _pos)) {
            return w;
        }
    }
    
    return nullptr;
}


#ifndef __WIDGET_TYPES_H
#define __WIDGET_TYPES_H

#include <string>
#include <functional>
#include <glm/glm.hpp>

// 
#define SYN_WINDOW_MAX_WIDGET_COUNT  16
#define SYN_TEXT_AREA_LINE_LEN      512

// 
enum class widget_type_t {
    BUTTON,
    LABEL,
    TEXT_AREA,
};

// 
enum class widget_anchor_t {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    CENTER,
};

// 
struct text_area_line_t {
    char text[SYN_TEXT_AREA_LINE_LEN];
    glm::vec4 color;
};

// 
struct widget_t {
    widget_type_t type;
    widget_anchor_t anchor;
    glm::vec2 position; // relative to anchor
    glm::vec2 size;
    glm::vec4 color;
    glm::vec4 hover_color;
    glm::vec4 outline_color;
    std::string text = "";
    bool is_enabled = true;
    bool is_hovered = false;
    bool is_visible = true;
    bool in_title_bar = false;
    
    // callbacks
    std::function<void()> on_click;
    // call: count = get_lines(text_area_lines_t *_out_buffer, uint32_t _max_lines)
    std::function<uint32_t(text_area_line_t *, uint32_t)> get_lines;
    std::function<void(widget_t *, const glm::vec2 &)> on_resize;
    
    // 
    widget_t() = default;

    // 
    glm::vec2 get_absolute_position(const glm::vec2 &_window_pos, const glm::vec2 &_window_size)
    {
        glm::vec2 abs_pos = _window_pos;
        switch (anchor) {
            case widget_anchor_t::TOP_LEFT: {
                abs_pos += position;
                break;
            }

            case widget_anchor_t::TOP_RIGHT: {
                abs_pos.x += _window_size.x - position.x - size.x;
                abs_pos.y += position.y;
                break;
            }

            case widget_anchor_t::BOTTOM_LEFT: {
                abs_pos.x += position.x;
                abs_pos.y += _window_size.y - position.y - size.y;
                break;
            }

            case widget_anchor_t::BOTTOM_RIGHT: {
                abs_pos.x += _window_size.x - position.x - size.x;
                abs_pos.y += _window_size.y - position.y - size.y;
                break;
            }

            case widget_anchor_t::CENTER: {
                abs_pos += (_window_size - size) * 0.5f + position;
                break;
            }

        }

        return abs_pos;
    }

    // 
    bool contains_point(const glm::vec2 _window_pos, const glm::vec2 &_window_size, 
                        const glm::vec2 &_p, float _title_bar_height)
    {
        glm::vec2 ref_pos, ref_size;
        if (in_title_bar) {
            ref_pos = _window_pos;
            ref_size = _window_size;
        } else {
            ref_pos = glm::vec2(_window_pos.x, _window_pos.y + _title_bar_height);
            ref_size = glm::vec2(_window_size.x, _window_size.y - _title_bar_height);
        }
     
        glm::vec2 p = get_absolute_position(ref_pos, ref_size);
        return (_p.x >= p.x && _p.x <= p.x + size.x &&
                _p.y >= p.y && _p.y <= p.y + size.y);
    }

};




#endif // __WIDGET_TYPES_H

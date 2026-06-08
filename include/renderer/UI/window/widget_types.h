#ifndef __WIDGET_TYPES_H
#define __WIDGET_TYPES_H

#include <string>
#include <functional>
#include <glm/glm.hpp>

// 
#define SYN_WINDOW_MAX_WIDGET_COUNT 16

// 
enum class widget_type_t {
    BUTTON,
    LABEL,
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

    // callback
    std::function<void()> on_click;

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
    bool contains_point(const glm::vec2 _window_pos, const glm::vec2 &_window_size, const glm::vec2 &_p)
    {
        glm::vec2 p = get_absolute_position(_window_pos, _window_size);
        return (_p.x >= p.x && _p.x <= p.x + size.x &&
                _p.y >= p.y && _p.y <= p.y + size.y);
    }

};




#endif // __WIDGET_TYPES_H

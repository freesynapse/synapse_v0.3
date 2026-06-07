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
struct widget_t {
    widget_type_t type;

    glm::vec2 position; // relative to window->position
    glm::vec2 size;

    glm::vec4 color;
    glm::vec4 hover_color;
    std::string text;

    // 
    bool is_enabled = true;
    bool is_hovered = false;
    bool is_visible = true;

    // callback
    std::function<void()> on_click;

    // 
    widget_t() = default;
    bool contains_point(const glm::vec2 _window_pos, const glm::vec2 &_p) {
        glm::vec2 p = _window_pos + position;
        return (_p.x >= p.x && _p.x <= p.x + size.x &&
                _p.y >= p.y && _p.y <= p.y + size.y);
    }
};




#endif // __WIDGET_TYPES_H

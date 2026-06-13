#ifndef __EVENT_H
#define __EVENT_H

#include <stdint.h>
#include <glm/glm.hpp>
#include "renderer/shader/shader_types.h"
#include "renderer/UI/window/window_types.h"

//
enum class event_type_t : uint8_t {
    NONE = 0,
    APPLICATION_EXIT,
    WINDOW_CLOSE, 
    WINDOW_RESIZE, 
    WINDOW_TOGGLE_FULLSCREEN, 
    WINDOW_TOGGLE_CURSOR, 
    WINDOW_TOGGLE_FROZEN_CURSOR, 
    UI_WINDOW_CLOSE, 
    INPUT_KEYDOWN, 
    INPUT_CHAR,
    INPUT_MOUSE_BUTTON, 
    INPUT_MOUSE_SCROLL,
    INPUT_MOUSE_MOVE,
    SHADER_RELOAD, 
    VIEWPORT_RESIZE,
};

inline static const char *str_event_type(event_type_t e)
{
    switch (e)
    {
    case event_type_t::NONE:                        return "event_type_t::NONE";
    case event_type_t::APPLICATION_EXIT:            return "event_type_t::APPLICATION_EXIT";
    case event_type_t::WINDOW_CLOSE:                return "event_type_t::WINDOW_CLOSE";
    case event_type_t::WINDOW_RESIZE:               return "event_type_t::WINDOW_RESIZE";
    case event_type_t::WINDOW_TOGGLE_FULLSCREEN:    return "event_type_t::WINDOW_TOGGLE_FULLSCREEN";
    case event_type_t::WINDOW_TOGGLE_CURSOR:        return "event_type_t::WINDOW_TOGGLE_CURSOR";
    case event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR: return "event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR";
    case event_type_t::UI_WINDOW_CLOSE:             return "event_type_t::UI_WINDOW_CLOSE";
    case event_type_t::INPUT_KEYDOWN:               return "event_type_t::INPUT_KEYDOWN";
    case event_type_t::INPUT_CHAR:                  return "event_type_t::INPUT_CHAR";
    case event_type_t::INPUT_MOUSE_BUTTON:          return "event_type_t::INPUT_MOUSE_BUTTON";
    case event_type_t::INPUT_MOUSE_SCROLL:          return "event_type_t::INPUT_MOUSE_SCROLL";
    case event_type_t::INPUT_MOUSE_MOVE:            return "event_type_t::INPUT_MOUSE_MOVE";
    case event_type_t::SHADER_RELOAD:               return "event_type_t::SHADER_RELOAD";
    case event_type_t::VIEWPORT_RESIZE:             return "event_type_t::VIEWPORT_RESIZE";
    default: return "Unknown event_type_t.";
    }
}

//
struct application_exit_event_t {
};

//
struct window_close_event_t {

};

//
struct window_resize_event_t {
    uint32_t width;
    uint32_t height;
};

//
struct toggle_fullscreen_event_t {
};

//
struct toggle_cursor_event_t {};

//
struct toggle_frozencursor_event_t {};

//
struct ui_window_close_event_t {
    window_handle_t handle;
};

//
struct keydown_event_t {
    int key;
    int action;
    int mods;
    window_handle_t focused_window_handle;
};

// 
struct input_char_event_t {
    uint32_t codepoint;
    window_handle_t focused_window_handle;
};

//
struct mouse_button_event_t {
    int button;
    int action;
    int mods;
    glm::vec2 pos;
    window_handle_t window_handle;
    
};

//
struct mouse_scroll_event_t {
    float xoffset;
    float yoffset;
    window_handle_t window_handle;

};

//
struct mouse_move_event_t {
    glm::vec2 pos;
    window_handle_t window_handle;

};

//
struct shader_reload_event_t {
    shader_handle_t handle;

};

//
struct viewport_resize_event_t {
    glm::ivec2 viewport;
    glm::vec2 fviewport;

};

//
struct event_t {
    event_type_t type;

    union {
        application_exit_event_t    application_exit;
        window_close_event_t        window_close;
        window_resize_event_t       window_resize;
        toggle_fullscreen_event_t   toggle_fullscreen;
        toggle_cursor_event_t       toggle_cursor;
        toggle_frozencursor_event_t toggle_frozencursor;
        ui_window_close_event_t     ui_window_close;
        keydown_event_t             keydown;
        input_char_event_t          input_char;
        mouse_button_event_t        mouse_button;
        mouse_scroll_event_t        mouse_scroll;
        mouse_move_event_t          mouse_move;
        shader_reload_event_t       shader_reload;
        viewport_resize_event_t     viewport_resize;
    } as;

    event_t() : type(event_type_t::NONE) {}
    ~event_t() {}

};



#endif // __EVENT_H

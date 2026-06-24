#ifndef __WIDGET_STATE_H
#define __WIDGET_STATE_H

#include <stdint.h>
#include <glm/glm.hpp>

#define SYN_IM_MAX_WIDGET_STATES    128
#define SYN_IM_BUFFER_LEN            64

// enum to handle dragging on im widgets
enum class im_drag_type_t {
    NONE = 0,
    COLOR_PICKER_SV,
    COLOR_PICKER_HUE
};

// im widget parameter setting
struct widget_params_t {
    glm::vec2 content_pos;
    glm::vec2 content_size;
    float pad;
    float row_h;

    float col_x;
    float col_w;
    bool in_row;

    glm::vec2 p; // widget position
    glm::vec2 s; // widget size
};

// per-widget state that must survive across frames
struct widget_state_t {
    uint32_t id                 = 0;        // hash of widget label
    bool is_hovered             = false;
    bool is_active              = false;    // e.g. a float field being edited
    bool is_dirty               = false;
    bool is_scrolling           = false;
    
    // float field
    float value                 = 0.0f;
    float *binding              = nullptr;
    float pre_scroll_value      = 0.0f;
    float min                   = -FLT_MAX;
    float max                   = FLT_MAX;
    int cursor                  = 0;
    bool editing                = false;
    char buf[SYN_IM_BUFFER_LEN] = {};
    
    // scroll
    float scroll_offset         = 0.0f;
    
};

#endif // __WIDGET_STATE_H

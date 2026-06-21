#ifndef __WIDGET_STATE_H
#define __WIDGET_STATE_H

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>

#define SYN_MAX_WIDGET_STATES 128

// per-widget state that must survive across frames
struct widget_state_t {
    uint32_t id         = 0;        // hash of widget label
    bool is_hovered     = false;
    bool is_active      = false;    // e.g. a float field being edited

    // float field
    float value         = 0.0f;
    char buf[32]        = {};
    int cursor          = 0;
    bool editing        = false;

    // scroll
    float scroll_offset = 0.0f;
    
};

// hash
inline uint32_t widget_hash(const char *_str)
{
    uint32_t hash = 2166136261u;
    while (*_str) {
        hash ^= (uint8_t)*_str++;
        hash *= 16777619u;
    }

    return hash ? hash : 1u;
    
}



#endif // __WIDGET_STATE_H

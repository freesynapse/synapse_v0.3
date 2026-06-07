#ifndef __WINDOW_TYPES_H
#define __WINDOW_TYPES_H

#include <string>
#include <glm/glm.hpp>

// 
struct window_handle_t {
    uint32_t id;

    // all shaders are stored as id > 0, so that 0 == invalid
    bool is_valid() const { return id != 0; }
    bool operator==(const window_handle_t &other) const { return id == other.id; }
};

// 
struct window_desc_t {
    glm::vec2 position;
    glm::vec2 size;
    
};








#endif // __WINDOW_TYPES_H

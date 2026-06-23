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
    std::string name;
    glm::vec2 position;
    glm::vec2 size;
    
};

// 
enum class resize_handle_t {
    NONE,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

// 
enum class dock_zone_t {
    NONE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER
};

struct dock_zone_visual_t {
    glm::vec4 bounds;
    dock_zone_t zone;
    bool is_hovered;
};






#endif // __WINDOW_TYPES_H

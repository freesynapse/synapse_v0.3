#ifndef __UI_TYPES_H
#define __UI_TYPES_H

#include <glm/glm.hpp>


// 
enum class ui_transform_mode_t { 
    TRANSLATE,
    ROTATE,
    SCALE
};

// 
enum class ui_transform_axis_t { 
    NONE,
    X,
    Y,
    Z 
};

// 
struct ui_transform_vertex_t {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;

    ui_transform_vertex_t(const glm::vec2 &_position, const glm::vec2 &_uv, const glm::vec4 &_color) :
        pos(_position), uv(_uv), color(_color) {}
    ui_transform_vertex_t() {}
};

// 
struct ui_render_vertex_t {
    glm::vec2 pos;
    glm::vec4 color;
    float depth;
    float pad;

    ui_render_vertex_t(const glm::vec2 &_position, const glm::vec4 &_color, float _depth) :
        pos(_position), color(_color), depth(_depth), pad(0.0f) {}
    ui_render_vertex_t() {};
    
};

// 
struct ui_tex_quad_vertex_t {
    glm::vec2 pos;
    glm::vec2 uv;
    float depth;
    float pad;

    ui_tex_quad_vertex_t(const glm::vec2 &_pos, const glm::vec2 &_uv, float _depth) :
        pos(_pos), uv(_uv), depth(_depth), pad(0.0f) {}
    ui_tex_quad_vertex_t() {}
};


#endif // __UI_TYPES_H

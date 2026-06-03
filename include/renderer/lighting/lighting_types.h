#ifndef __LIGHTING_TYPES_H
#define __LIGHTING_TYPES_H

#include <glm/glm.hpp>

// 
#define SYN_MAX_LIGHTS 16

// 
struct light_t {
    glm::vec4 position; // .w = type [0: point light, 1: directional light, 2:spotlight]
    glm::vec4 color;
    glm::vec4 direction;
    glm::vec4 params;   // [inner_cutoff, outer_cutoff, radius, _pad]
};


// global lighting context matching GLSL std140 layout
struct light_internal_t {
    light_t lights[SYN_MAX_LIGHTS];     // 64 x MAX_LIGHTS bytes
    uint32_t light_count;               //  4 bytes
    float padding[3];                   // 12 bytes
};




#endif // __LIGHTING_TYPES_H

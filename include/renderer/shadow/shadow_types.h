#ifndef __SHADOW_TYPES_H
#define __SHADOW_TYPES_H

#include <glm/glm.hpp>

// 
#define SYN_SHADOW_MAP_SIZE 2048

// 
struct shadow_map_t {
    GLuint fbo_id   = 0;
    GLuint depth_id = 0;
    bool is_active  = false;

    glm::mat4 light_space_matrix = glm::mat4(1.0f);

    float ortho_size = 20.0f;
    float z_near     = 0.1f;
    float z_far      = 100.0f;
    
};


#endif // __SHADOW_TYPES_H

#ifndef __SHADER_TYPES_H
#define __SHADER_TYPES_H

#include <stdint.h>
#include "external/glad/glad.h"

// 
#define SYN_MAX_UNIFORM_NAME_LEN    64
#define SYN_MAX_SHADER_UNIFORMS     32

// 
struct uniform_cache_t {
    char name[SYN_MAX_UNIFORM_NAME_LEN];
    GLint location;
};

// 
struct shader_handle_t {
    uint32_t id; // = 0;

    // all shaders are stored as id > 0, so that 0 == invalid
    bool is_valid() const { return id != 0; }
    bool operator==(const shader_handle_t &other) const { return id == other.id; }    
};








#endif // __SHADER_TYPES_H

#ifndef __TEXTURE_TYPES_H
#define __TEXTURE_TYPES_H

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>

// 
// #define MAX_TEXTURE_PATH_LEN    128
#define SYN_MAX_TEXTURE_COUNT    64

struct texture_handle_t {
    uint32_t id = 0;
    bool is_valid() { return id != 0; }
    bool operator==(const texture_handle_t &_other) const { return id == _other.id; }
};

// 
struct texture_internal_t {
    uint32_t opengl_id = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;
    std::string asset_path;
    // char asset_path[MAX_TEXTURE_PATH_LEN] = { 0 };
    bool is_active = false;
};


#endif // __TEXTURE_TYPES_H

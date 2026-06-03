#ifndef __CUBEMAP_TYPES_H
#define __CUBEMAP_TYPES_H

#include <stdint.h>

#include "renderer/shader/shader_types.h"
#include "renderer/mesh/mesh_types.h"


// 
struct cubemap_handle_t {
    uint32_t id = 0;
    bool is_valid() const { return id != 0; }
    bool operator==(const cubemap_handle_t &_other) { return id == _other.id; }
};

struct cubemap_internal_t {
    uint32_t opengl_id = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool is_active = false;
};

struct skybox_t {
    cubemap_handle_t cubemap_handle = { 0 };
    shader_handle_t shader_handle = { 0 };
    mesh_handle_t mesh_handle { 0 };
    bool is_active = false;
};

#endif // __CUBEMAP_TYPES_H

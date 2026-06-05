#ifndef __MESH_TYPES_H
#define __MESH_TYPES_H

#include <string>
#include <glm/glm.hpp>

#include "renderer/buffers/vertex_array.h"

// 
struct vertex_data_t
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;
};

// 
struct mesh_handle_t {
    uint32_t id = 0;
    bool is_valid() { return id != 0; }
    bool operator==(const mesh_handle_t &other) const { return id == other.id; }
};

// 
struct mesh_internal_t {
    vertex_array_t vao;
    glm::vec3 aabb_min = glm::vec3(0.0f);
    glm::vec3 aabb_max = glm::vec3(0.0f);
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;
    std::string name;
    bool is_active = false;
    
};

#endif // __MESH_TYPES_H

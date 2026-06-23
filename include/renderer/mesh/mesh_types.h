#ifndef __MESH_TYPES_H
#define __MESH_TYPES_H

#include <string>
#include <glm/glm.hpp>

#include "renderer/buffers/vertex_array.h"


// 
enum class primitive_type_t : uint8_t {
    NONE = 0,
    CUBE,
    SPHERE_UV,
    PLANE,
    CONE,
    CYLINDER,
    TORUS,
    COUNT,
};

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

// 2D "meshes" are kept here
struct vertex_data_2d_t {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 color;
    float tex_index;
    float depth;
};

struct transform_2d_t {
    glm::vec2 position;
    glm::vec2 scale;
    float rotation; // in radians
};

struct quad_2d_t {
    transform_2d_t transform;
    glm::vec4 color;
    float tex_index;
    float depth;
};


#endif // __MESH_TYPES_H

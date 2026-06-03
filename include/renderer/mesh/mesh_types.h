#ifndef __MESH_TYPES_H
#define __MESH_TYPES_H

#include <glm/glm.hpp>

// TODO : add tangent (and bitangents?)
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


#endif // __MESH_TYPES_H

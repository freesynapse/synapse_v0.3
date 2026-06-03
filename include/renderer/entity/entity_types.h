#ifndef __ENTITY_TYPES_H
#define __ENTITY_TYPES_H

#include <string>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/mesh/mesh_types.h"
#include "renderer/material/material_types.h"

// 
struct entity_handle_t {
    uint32_t id = 0;
    bool is_valid() { return id != 0; }
    bool operator==(const entity_handle_t &_other) { return id == _other.id; }

};

// 
struct entity_t {
    std::string name;
    mesh_handle_t mesh_handle;
    material_handle_t material_handle;
    glm::mat4 transform = glm::mat4(1.0f);

    bool is_active = false;

    entity_t() :
        mesh_handle({ 0 }), material_handle({ 0 }), transform(1.0f)
    {}

    // construct transform from components
    static glm::mat4 make_transform(const glm::vec3 &_position, 
                                    const glm::vec3 &_rotation_degrees, // Euler angles in degrees
                                    const glm::vec3 &_scale)
    {
        glm::mat4 transform(1.0f);

        // apply transformations : T x R x S
        transform = glm::translate(transform, _position);
        transform = glm::rotate(transform, glm::radians(_rotation_degrees.y), glm::vec3(0, 1, 0)); // y
        transform = glm::rotate(transform, glm::radians(_rotation_degrees.x), glm::vec3(1, 0, 0)); // x
        transform = glm::rotate(transform, glm::radians(_rotation_degrees.z), glm::vec3(0, 0, 1)); // z
        transform = glm::scale(transform, _scale);

        return transform;
    }
    
};


#endif // __ENTITY_TYPES_H

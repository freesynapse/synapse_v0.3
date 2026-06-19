#ifndef __ENTITY_TYPES_H
#define __ENTITY_TYPES_H

#include <string>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
    primitive_type_t mesh_primitive_type = primitive_type_t::NONE;
    float mesh_params[4] = { 0.0f };
    uint32_t mesh_param_count = 0;
    material_handle_t material_handle;
    std::string manifest_material_name;
    glm::mat4 transform = glm::mat4(1.0f);

    glm::vec3 t_position = glm::vec3(0.0f);
    glm::vec3 t_rotation = glm::vec3(0.0f);
    glm::vec3 t_scale    = glm::vec3(1.0f);
    
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

// 
struct transfrom_components_t {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);   // Euler degrees
    glm::vec3 scale    = glm::vec3(0.0f, 0.0f, 0.0f);
};

inline transfrom_components_t decompose_transorm(const glm::mat4 &_m)
{
    transfrom_components_t out;
    glm::vec3 scew;
    glm::vec4 persp;
    glm::quat orientation;
    glm::decompose(_m, out.scale, orientation, out.position, scew, persp);
    out.rotation = glm::degrees(glm::eulerAngles(orientation));
    return out;
}


#endif // __ENTITY_TYPES_H


#include <string.h>

#include "renderer/entity/entity_library.h"
#include "utils/log.h"

// 
void entity_library_t::init()
{
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        m_pool[i] = entity_t();
    }
    m_active_count = 0;
}

// 
entity_handle_t entity_library_t::create_entity(const std::string &_name,
                                                mesh_handle_t _mesh_handle,
                                                material_handle_t _material_handle,
                                                const glm::mat4 &_transform)
{
    uint32_t slot = SYN_MAX_ENTITY_COUNT;
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        if (!m_pool[i].is_active) {
            slot = i;
            break;
        }
    }

    if (slot == SYN_MAX_ENTITY_COUNT) {
        SYN_ERROR("entity count >= SYN_MAX_ENTITY_COUNT.\n");
        return { 0 };
    }

    entity_t &ent = m_pool[slot];
    ent.name = _name;
    ent.mesh_handle = _mesh_handle;
    ent.material_handle = _material_handle;
    ent.transform = _transform;
    ent.is_active = true;

    ent.t_position = glm::vec3(_transform[3]);
    ent.t_scale = glm::vec3(
        glm::length(glm::vec3(_transform[0])),
        glm::length(glm::vec3(_transform[1])),
        glm::length(glm::vec3(_transform[2])));
    glm::mat3 rot_matrix = glm::mat3(
        glm::vec3(_transform[0]) / ent.t_scale.x,
        glm::vec3(_transform[1]) / ent.t_scale.y,
        glm::vec3(_transform[2]) / ent.t_scale.z);
    ent.t_rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rot_matrix)));
    
    m_active_count++;
    
    return { slot + 1 };
    
}

// 
entity_handle_t entity_library_t::add_entity(const entity_t &_entity)
{
    return create_entity(_entity.name, _entity.mesh_handle, _entity.material_handle, _entity.transform);
    
}


// 
entity_t *entity_library_t::get_entity(entity_handle_t _handle)
{
    uint32_t idx = _handle.id - 1;
    if (_handle.id == 0 || idx >= SYN_MAX_ENTITY_COUNT) {
        SYN_WARNING("invalid entity_handle_t: id = %d.\n", _handle.id);
        return nullptr;
    }

    return &m_pool[idx];
}

// 
entity_t *entity_library_t::get_entity_from_index(uint32_t _index)
{
    if (_index >= SYN_MAX_ENTITY_COUNT) {
        SYN_WARNING("invalid index = %d.\n", _index);
        return nullptr;
    }

    return &m_pool[_index];
}

// 
void entity_library_t::release_entity(entity_handle_t _handle)
{
    if (_handle.id > 0 && _handle.id < SYN_MAX_ENTITY_COUNT) {
        entity_t &ent = m_pool[_handle.id - 1];
        if (ent.is_active) {
            ent = entity_t();
            m_active_count--;
        }
    }
}
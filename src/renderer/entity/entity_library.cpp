
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
    // search for duplicates
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        if (m_pool[i].is_active && _name == m_pool[i].name) {
            return { i + 1 };
        }
    }

    // check overflow
    if (m_active_count >= SYN_MAX_ENTITY_COUNT) {
        SYN_ERROR("entity count >= SYN_MAX_ENTITY_COUNT.\n");
        return { 0 };
    }

    uint32_t slot = m_active_count;
    entity_t &ent = m_pool[slot];
    ent.name = _name;
    ent.mesh_handle = _mesh_handle;
    ent.material_handle = _material_handle;
    ent.transform = _transform;
    ent.is_active = true;

    transfrom_components_t tc = decompose_transorm(_transform);
    ent.t_position = tc.position;
    ent.t_rotation = tc.rotation;
    ent.t_scale    = tc.scale;
    
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

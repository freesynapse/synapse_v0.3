#ifndef __ENTITY_LIBRARY_H
#define __ENTITY_LIBRARY_H

#include "renderer/entity/entity_types.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/material/material_types.h"

// 
#define SYN_MAX_ENTITY_COUNT    256

// 
class entity_library_t
{
public:
    friend class renderer_t;
    
public:
    void init();

    entity_handle_t create_entity(const std::string &_name,
                                  mesh_handle_t _mesh_handle,
                                  material_handle_t _material_handle,
                                  const glm::mat4 &_transform);
    entity_handle_t add_entity(const entity_t &_entity);
    entity_t *get_entity(entity_handle_t _handle);

private:
    entity_t m_pool[SYN_MAX_ENTITY_COUNT];
    uint32_t m_active_count = 0;
    
};


#endif // __ENTITY_LIBRARY_H

#ifndef __MATERIAL_LIBRARY_H
#define __MATERIAL_LIBRARY_H

#include "renderer/material/material_types.h"
#include "renderer/shader/shader_types.h"

// 
class material_library_t
{
public:
    void init();
    void shutdown();

    material_handle_t create_material(shader_handle_t _shader_handle);
    material_internal_t *get_material(material_handle_t _handle);
    void release_material(material_handle_t _handle);

    material_internal_t m_pool[SYN_MAX_MATERIAL_COUNT];
    uint32_t m_active_count;
    
};


#endif // __MATERIAL_LIBRARY_H

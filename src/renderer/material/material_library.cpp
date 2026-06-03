
#include <string.h>

#include "renderer/material/material_library.h"
#include "utils/log.h"

// 
void material_library_t::init()
{
    memset(m_pool, 0, sizeof(material_internal_t) * SYN_MAX_MATERIAL_COUNT);
    m_active_count = 0;
}

// 
void material_library_t::shutdown()
{
    init();
}

// 
material_handle_t material_library_t::create_material(uint32_t _shader_program_id)
{
    uint32_t free_slot = 0;

    // find free memory; as usual, index 0 is reserved as an invalid material
    for (uint32_t i = 1; i < SYN_MAX_MATERIAL_COUNT; i++) {
        if (!m_pool[i].is_active) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == 0) {
        SYN_WARNING("SYN_MAX_MATERIAL_COUNT reached. New material creation rejected.\n");
        return { 0 };
    }

    material_internal_t &mat = m_pool[free_slot];
    mat.data_size = sizeof(material_pbr_payload_t);
    material_pbr_payload_t *pbr_payload = (material_pbr_payload_t *)mat.data;
    // setup initial state
    pbr_payload->albedo_color        = glm::vec4(1.0f);
    pbr_payload->emissive_color      = glm::vec4(0.0f);
    pbr_payload->roughness           = 0.5f;
    pbr_payload->metallic            = 0.0f;
    pbr_payload->ao                  = 1.0f;
    pbr_payload->tiling_factor       = 1.0f;
    pbr_payload->use_albedo_map      = 0.0f;
    pbr_payload->use_normal_map      = 0.0f;
    pbr_payload->use_metallic_map    = 0.0f;
    pbr_payload->use_roughness_map   = 0.0f;

    // 
    for (uint32_t i = 0; i < (uint32_t)texture_map_type_t::COUNT; i++) {
        mat.textures[i] = { 0 };
    }

    mat.shader_program_id = _shader_program_id;
    mat.is_active = true;
    m_active_count++;

    #ifdef DEBUG_MATERIAL_LIBRARY
    SYN_INFO("loaded material %d.\n", free_slot);
    #endif
    
    return { free_slot };
    
}

// 
material_internal_t *material_library_t::get_material(material_handle_t _handle)
{
    if (_handle.id == 0 || _handle.id >= SYN_MAX_MATERIAL_COUNT || !m_pool[_handle.id].is_active) {
        return nullptr;
    }
    
    return &m_pool[_handle.id];
}

// 
void material_library_t::release_material(material_handle_t _handle)
{
    if (_handle.id > 0 && _handle.id < SYN_MAX_MATERIAL_COUNT) {
        material_internal_t &mat = m_pool[_handle.id];
        if (mat.is_active) {
            memset(&mat, 0, sizeof(material_internal_t));
            m_active_count--;
        }
    }
}


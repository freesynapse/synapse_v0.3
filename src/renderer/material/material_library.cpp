
#include <string.h>

#include "renderer/material/material_library.h"
#include "utils/log.h"

#include "c_api.h"

// 
void material_library_t::init()
{
    memset(m_pool, 0, sizeof(material_internal_t) * SYN_MAX_MATERIAL_COUNT);
    m_active_count = 0;

    create_fallback_material();
    SYN_INFO("fallback material handle id = %d\n", fallback_material_handle.id);
    
}

// 
void material_library_t::shutdown()
{
    init();
}

// 
material_handle_t material_library_t::create_material(shader_handle_t _shader_handle)
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

    mat.shader_handle = _shader_handle;
    mat.is_active = true;
    m_active_count++;

    return { free_slot };
    
}

// 
material_handle_t material_library_t::create_material_from(material_handle_t _handle)
{
    material_internal_t *src = get_material(_handle);
    material_handle_t new_handle = create_material(src->shader_handle);
    material_internal_t *dst = get_material(new_handle);

    if (src && dst) {
        memcpy(dst->data, src->data, src->data_size);
        dst->data_size = src->data_size;
        memcpy(dst->textures, src->textures, sizeof(src->textures));
    }

    return new_handle;
}

// 
material_internal_t *material_library_t::get_material(material_handle_t _handle)
{
    if (_handle.id == 0 || _handle.id >= SYN_MAX_MATERIAL_COUNT || !m_pool[_handle.id].is_active) {
        if (_handle.id == fallback_material_handle.id) return nullptr;
        
        return get_material(fallback_material_handle);
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

// 
void material_library_t::create_fallback_material()
{
    shader_handle_t pbr_shader = shader_lib.load_from_file("pbr_shader", "../assets/shaders/PBR_IBL.glsl");

    // create material
    fallback_material_handle = mat_lib.create_material(pbr_shader);
    material_internal_t *fb_mat = mat_lib.get_material(fallback_material_handle);

    if (fb_mat) {
        material_pbr_payload_t *pbr = (material_pbr_payload_t *)fb_mat->data;
        pbr->albedo_color   = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pbr->roughness      = 0.26f;
        pbr->metallic       = 0.25f;
        pbr->ao             = 1.00f;
        pbr->tiling_factor  = 1.00f;
        pbr->use_albedo_map = 0.00f;
    }
    
}


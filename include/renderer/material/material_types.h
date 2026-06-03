#ifndef __MATERIAL_TYPES_H
#define __MATERIAL_TYPES_H

#include <stdint.h>
#include <string.h>
#include <glm/glm.hpp>
#include "renderer/material/texture_types.h"

// 
#define SYN_MAX_MATERIAL_COUNT       64
#define SYN_MAX_MATERIAL_DATA_SIZE  256  // enough for a large UBO
#define SYN_MAX_MATERIAL_TEXTURES     8

// 
enum class texture_map_type_t : uint32_t {
    ALBEDO      = 0,
    NORMAL      = 1,
    METALLIC    = 2,
    ROUGHNESS   = 3,
    AO          = 4,
    EMISSIVE    = 5,
    COUNT       = 6,
};

// 
struct material_handle_t {
    uint32_t id = 0;
    bool is_valid() { return id != 0; }
    bool operator==(const material_handle_t &_other) { return id == _other.id; }
};

// matching std140 UBO
struct material_pbr_payload_t {
    glm::vec4 albedo_color      = glm::vec4(1.0f);
    glm::vec4 emissive_color    = glm::vec4(0.0f);

    float roughness             = 0.5f;
    float metallic              = 0.0f;
    float ao                    = 1.0f;
    float tiling_factor         = 1.0f;

    // flags
    float use_albedo_map        = 0.0f;
    float use_normal_map        = 0.0f;
    float use_metallic_map      = 0.0f;
    float use_roughness_map     = 0.0f;
    float use_ao_map            = 0.0f;
    float use_emissive_map      = 0.0f;

    float _pad[2];
};

// 
//struct material_internal_t {
//    material_payload_t payload;
//    texture_handle_t textures[(uint32_t)texture_map_type_t::COUNT];
//    uint32_t shader_program_id;
//    bool is_active;
//
//    material_internal_t() {
//        for (uint32_t i = 0; i < (uint32_t)texture_map_type_t::COUNT; i++) {
//            textures[i] = { 0 };
//        }
//    }
//};
struct material_internal_t {
    uint32_t shader_program_id;

    // generic data (payload) storage
    uint8_t data[SYN_MAX_MATERIAL_DATA_SIZE];
    uint32_t data_size;

    // generic texture slots
    texture_handle_t textures[SYN_MAX_MATERIAL_TEXTURES];

    bool is_active;

    material_internal_t() :
        shader_program_id(0), data_size(0), is_active(false)
    {
        memset(data, 0, SYN_MAX_MATERIAL_DATA_SIZE);
        for (uint32_t i = 0; i < SYN_MAX_MATERIAL_TEXTURES; i++) textures[i] = { 0 };
    }
};


#endif // __MATERIAL_TYPES_H

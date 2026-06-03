#ifndef __ASSET_MANAGER_H
#define __ASSET_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "renderer/shader/shader_types.h"
#include "renderer/material/texture_types.h"
#include "renderer/material/material_types.h"
#include "renderer/material/cubemap_types.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/entity/entity_types.h"

// 
struct material_descriptor_t
{
    std::string shader_name;
    
    // texture slots (empty string = "not specified")
    std::string albedo_texture;
    std::string normal_texture;
    std::string metallic_texture;
    std::string roughness_texture;
    std::string ao_texture;
    std::string emissive_texture;
    
    // PBR flags (use_*_map)
    bool use_albedo_map = false;
    bool use_normal_map = false;
    bool use_metallic_map = false;
    bool use_roughness_map = false;
    bool use_ao_map = false;
    bool use_emissive_map = false;
    
    // material properties
    glm::vec4 albedo_color = glm::vec4(1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float tiling_factor = 1.0f;
};

// 
struct entity_descriptor_t {
    std::string mesh_name;
    std::string material_name;

    glm::vec3 position          = glm::vec3(0.0f);
    glm::vec3 rotation_degrees  = glm::vec3(0.0f);
    glm::vec3 scale             = glm::vec3(0.0f);
    
};

// 
class asset_manager_t
{
public:
    void init();
    void shutdown();

    // Loads a manifest file and populates all libraries
    void load_manifest(const std::string &_path="./assets.syn");

    // High-level accessors
    shader_handle_t   get_shader(const std::string &_name);
    texture_handle_t  get_texture(const std::string &_name);
    material_handle_t get_material(const std::string &_name);
    mesh_handle_t     get_mesh(const std::string &_name);
    cubemap_handle_t  get_skybox(const std::string &_name);
    entity_handle_t   get_entity(const std::string &_name);

private:
    void parse_line(const std::string &_line, size_t _pass);

    void parse_shader(const std::vector<std::string> &_tokens);
    void parse_texture(const std::vector<std::string> &_tokens);
    void parse_mesh(const std::vector<std::string> &_tokens);

    void parse_material(const std::vector<std::string> &_tokens);
    void parse_material_block(const std::string &_block, const std::string &_mat_name);
    void parse_material_property(material_descriptor_t &_desc, const std::string &_key, const std::string &_value, const std::string &_mat_name);
    void apply_smart_defaults(material_descriptor_t &_desc);
    void create_material_from_descriptor(const std::string &_mat_name, material_descriptor_t &_desc);
    void assign_texture_slot(material_internal_t *_mat, size_t _slot, const std::string &_tex_name);

    void parse_skybox(const std::vector<std::string> &_tokens);

    void parse_entity(const std::string &_block, const std::string &_entity_name);
    void parse_entity_property(entity_descriptor_t &_desc, const std::string &_key, const std::string &_value, const std::string &_entity_name);
    void create_entity_from_descriptor(const std::string &_name, const entity_descriptor_t &_desc);

    bool parse_bool(const std::string &_value);
    float parse_float(const std::string &_value);
    glm::vec3 parse_vec3(const std::string &_value);
    glm::vec4 parse_vec4(const std::string &_value);

    //
    void count_assets_in_manifest(const std::vector<std::string> &_lines);
    void update_load_progress(const std::string _type, const std::string &_name);
    void render_loading_assets();

private:    
    // internal handle maps for named lookup
    // note: the libraries themselves might already have these, 
    // but a centralized map here ensures consistent named access.
    std::unordered_map<std::string, shader_handle_t>   m_shader_map;
    std::unordered_map<std::string, texture_handle_t>  m_texture_map;
    std::unordered_map<std::string, material_handle_t> m_material_map;
    std::unordered_map<std::string, mesh_handle_t>     m_mesh_map;
    std::unordered_map<std::string, cubemap_handle_t>  m_cubemap_map;
    std::unordered_map<std::string, entity_handle_t>   m_entity_map;

    struct {
        uint32_t total_assets = 0;
        uint32_t loaded_assets = 0;
        std::string current_asset = "";
        bool enabled = false;
    } m_load_progress;
};


#endif // __ASSET_MANAGER_H

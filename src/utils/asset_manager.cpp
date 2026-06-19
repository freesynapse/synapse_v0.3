
#include <fstream>
#include <sstream>
#include <vector>

#include "utils/asset_manager.h"
#include "utils/log.h"

#include "c_api.h"

//
static std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

//
void asset_manager_t::load_manifest(const std::string &_path)
{
    m_current_manifest_path = _path;

    std::ifstream file(_path);

    if (!file.is_open()) {
        SYN_ERROR("failed to open manifest file: %s.\n", _path.c_str());
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Check if this starts a block (peek next line for '{')
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));

        std::streampos pos = file.tellg();  // Save position
        std::string next_line;
        bool is_block = false;

        if (std::getline(file, next_line)) {
            next_line.erase(0, next_line.find_first_not_of(" \t"));
            if (next_line[0] == '{') {
                is_block = true;
            }
            file.seekg(pos);  // Restore position
        }

        if (is_block || line.find('{') != std::string::npos) {
            // Read entire block
            std::string block = line + "\n";
            while (std::getline(file, line)) {
                block += line + "\n";
                if (line.find('}') != std::string::npos) break;
            }
            lines.push_back(block);
        } else {
            lines.push_back(line);
        }
    }

    file.close();

    count_assets_in_manifest(lines);
    m_load_progress.loaded_assets = 0;
    m_load_progress.enabled = true;

    // 1st pass: shaders, textures, meshes
    SYN_INFO("loading shaders, textures and meshes.\n");
    for (const auto &line : lines) parse_line(line, 1);

    // 2nd pass: materials, skybox
    SYN_INFO("loading materials and cubemaps.\n");
    for (const auto &line : lines) parse_line(line, 2);

    SYN_INFO("loading entities.\n");
    for (const auto &line : lines) parse_line(line, 3);

    m_load_progress.current_asset = "";
    m_load_progress.loaded_assets++;
    render_loading_assets();

}

//
void asset_manager_t::reload_manifest()
{
    if (m_current_manifest_path.empty()) {
        SYN_WARNING("no manifest loaded to reload.\n");
        return;
    }

    SYN_INFO("reloading manifest: %s.\n", m_current_manifest_path.c_str());

    load_manifest(m_current_manifest_path);

}

//
shader_handle_t asset_manager_t::get_shader(const std::string &_name)
{
    auto it = m_shader_map.find(_name);
    if (it == m_shader_map.end()) {
        SYN_ERROR("shader '%s' not loaded.\n", _name.c_str());
        return { 0 };
    }
    return it->second;

}

//
texture_handle_t asset_manager_t::get_texture(const std::string &_name)
{
    auto it = m_texture_map.find(_name);
    if (it == m_texture_map.end()) {
        SYN_ERROR("texture '%s' not loaded.\n", _name.c_str());
        return { 0 };
    }
    return it->second;

}

//
material_handle_t asset_manager_t::get_material(const std::string &_name)
{
    auto it = m_material_map.find(_name);
    if (it == m_material_map.end()) {
        SYN_ERROR("material '%s' not loaded, using fallback material.\n", _name.c_str());
        return mat_lib.fallback_material_handle;
    }

    return it->second;

}

//
std::string asset_manager_t::get_material_name(material_handle_t _handle)
{
    for (auto &pair : m_material_map)
        if (pair.second.id == _handle.id)
            return pair.first;
    return "";
}

//
mesh_handle_t asset_manager_t::get_mesh(const std::string &_name)
{
    auto it = m_mesh_map.find(_name);
    if (it == m_mesh_map.end()) {
        SYN_ERROR("mesh '%s' not loaded.\n", _name.c_str());
        return { 0 };
    }
    return it->second;

}

//
cubemap_handle_t asset_manager_t::get_skybox(const std::string &_name)
{
    auto it = m_cubemap_map.find(_name);
    if (it == m_cubemap_map.end()) {
        SYN_ERROR("skybox '%s' not loaded.\n", _name.c_str());
        return { 0 };
    }
    return it->second;

}

//
entity_handle_t asset_manager_t::get_entity(const std::string &_name)
{
    auto it = m_entity_map.find(_name);
    if (it == m_entity_map.end()) {
        SYN_ERROR("entity '%s' not loaded.\n", _name.c_str());
        return { 0 };
    }
    return it->second;
}

// 
std::string asset_manager_t::get_entity_name(entity_handle_t _handle)
{
    for (auto &pair : m_entity_map) {
        if (pair.second.id == _handle.id) {
            return pair.first;
        }
    }
    return "";
    
}

// 
mesh_handle_t asset_manager_t::get_entity_mesh(const std::string &_name)
{
    // look up entity descriptor's mesh name, then get mesh handle
    auto it = m_entity_mesh_map.find(_name);
    if (it == m_entity_mesh_map.end()) {
        SYN_ERROR("no mesh found for entity '%s'.\n", _name.c_str());
        return { 0 };
    }
    return it->second;
}

//
void asset_manager_t::parse_line(const std::string &_line, size_t _pass)
{
    bool is_block = (_line.find('\n') != std::string::npos);

    std::string first_line = _line.substr(0, _line.find('\n'));
    std::istringstream iss(first_line);
    std::vector<std::string> tokens;
    std::string token;


    // tokenize by whitespace (' ')
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return;

    const std::string &type = tokens[0];

    if (type == "shader" && _pass == 1) parse_shader(tokens);
    else if (type == "texture" && _pass == 1) parse_texture(tokens);
    else if (type == "mesh" && _pass == 1) parse_mesh(tokens);
    else if (type == "material" && _pass == 2) {
        if (is_block) {
            parse_material_block(_line, tokens[1]);
        } else {
            parse_material(tokens);
        }
    }
    else if (type == "skybox" && _pass == 2) parse_skybox(tokens);

    else if (type == "entity" && _pass == 3) {
        if (is_block) {
            parse_entity(_line, tokens[1]);
        } else {
            SYN_WARNING("entity '%s' must use block syntax.\n", tokens[1].c_str());
        }
    }
}

//
void asset_manager_t::parse_shader(const std::vector<std::string> &_tokens)
{
    // format: shader <name> <path>
    if (_tokens.size() < 3) {
        SYN_ERROR("invalid shader entry: expected 'shader <name> <path>'.\n");
        return;
    }

    const std::string &name = _tokens[1];
    const std::string &path = _tokens[2];

    update_load_progress("shader", name);

    shader_handle_t handle = shader_lib.load_from_file(name, path);
    m_shader_map[name] = handle;

    SYN_INFO("loaded shader '%s' from '%s'.\n", name.c_str(), path.c_str());

}

//
void asset_manager_t::parse_texture(const std::vector<std::string> &_tokens)
{
    // format: texture <name> <path>
    if (_tokens.size() < 3) {
        SYN_ERROR("invalid texture entry: expected 'texture <name> <path>'.\n");
        return;
    }

    const std::string &name = _tokens[1];
    const std::string &path = _tokens[2];

    update_load_progress("texture", name);

    texture_handle_t handle = tex_lib.load_texture(path);
    texture_internal_t *tex = tex_lib.get_texture(handle);
    if (tex) tex->name = name;
    m_texture_map[name] = handle;

    SYN_INFO("loaded texture '%s' from '%s'.\n", name.c_str(), path.c_str());

}

//
void asset_manager_t::parse_mesh(const std::vector<std::string> &_tokens)
{
    // format: mesh <name> <path>
    if (_tokens.size() < 3) {
        SYN_ERROR("invalid mesh entry: expected 'mesh <name> <path>'.\n");
        return;
    }

    const std::string &name = _tokens[1];
    const std::string &path = _tokens[2];

    update_load_progress("mesh", name);

    mesh_handle_t handle = mesh_lib.load_mesh_from_file(path);
    m_mesh_map[name] = handle;

    SYN_INFO("loaded mesh '%s' from '%s'.\n", name.c_str(), path.c_str());

}

//
void asset_manager_t::parse_material(const std::vector<std::string> &_tokens)
{
    // format: material <name> <shader_name> <tex0> <tex1> ... <texN>
    if (_tokens.size() < 3) {
        SYN_ERROR("invalid material entry: expected 'material <name> <shader_name> <tex0> <tex1> ... <texN>'\n");
        return;
    }

    const std::string& mat_name = _tokens[1];
    const std::string& shader_name = _tokens[2];

    update_load_progress("material", mat_name);

    // check shader
    auto shader_it = m_shader_map.find(shader_name);
    if (shader_it == m_shader_map.end()) {
        SYN_ERROR("material '%s' references unknown shader '%s'.\n", mat_name.c_str(), shader_name.c_str());
        return;
    }

    // create the material
    material_handle_t mat_handle = mat_lib.create_material(shader_it->second);
    material_internal_t *mat = mat_lib.get_material(mat_handle);

    if (!mat) {
        SYN_ERROR("failed to create material '%s'.\n", mat_name.c_str());
        return;
    }

    // process textures
    for (size_t i = 3; i < _tokens.size(); i++) {
        const std::string &tex_name = _tokens[i];
        size_t idx = i - 3;

        if (tex_name == "none") {
            mat->textures[idx] = { 0 };
        } else {
            auto tex_it = m_texture_map.find(tex_name);
            if (tex_it != m_texture_map.end()) {
                mat->textures[idx] = tex_it->second;
            } else {
                SYN_WARNING("material '%s' references unknown texture '%s'.\n", mat_name.c_str(), tex_name.c_str());
                mat->textures[idx] = { 0 };
            }
        }
    }

    m_material_map[mat_name] = mat_handle;
    SYN_INFO("created material '%s' with shader '%s'.\n", mat_name.c_str(), shader_name.c_str());

}

//
void asset_manager_t::parse_material_block(const std::string &_block, const std::string &_mat_name)
{
    material_descriptor_t desc;
    std::istringstream block_stream(_block);
    std::string line;

    update_load_progress("shader", _mat_name);

    // skip first line (material name)
    std::getline(block_stream, line);

    while (std::getline(block_stream, line)) {
        // trim whitespace
        line = trim(line);

        // skip empty lines
        if (line.empty() || line[0] == '#' || line[0] == '{' || line[0] == '}') continue;

        // parse key-value
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            SYN_WARNING("invalid material property line (colon missing): '%s'.\n", line.c_str());
            continue;
        }

        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        // trim key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        // remove trailing ','
        if (!value.empty() && value.back() == ',') {
            value.pop_back();
        }

        parse_material_property(desc, key, value, _mat_name);

    }

    apply_smart_defaults(desc);

    create_material_from_descriptor(_mat_name, desc);

}

//
void asset_manager_t::parse_material_property(material_descriptor_t &_desc,
                                              const std::string &_key,
                                              const std::string &_value,
                                              const std::string &_mat_name)
{
    // Shader reference
    if (_key == "shader") {
        _desc.shader_name = _value;
    }

    // texture slots
    else if (_key == "albedo_texture")    _desc.albedo_texture = _value;
    else if (_key == "normal_texture")    _desc.normal_texture = _value;
    else if (_key == "metallic_texture")  _desc.metallic_texture = _value;
    else if (_key == "roughness_texture") _desc.roughness_texture = _value;
    else if (_key == "ao_texture")        _desc.ao_texture = _value;
    else if (_key == "emissive_texture")  _desc.emissive_texture = _value;

    // flags
    else if (_key == "use_albedo_map")    _desc.use_albedo_map = parse_bool(_value);
    else if (_key == "use_normal_map")    _desc.use_normal_map = parse_bool(_value);
    else if (_key == "use_metallic_map")  _desc.use_metallic_map = parse_bool(_value);
    else if (_key == "use_roughness_map") _desc.use_roughness_map = parse_bool(_value);
    else if (_key == "use_ao_map")        _desc.use_ao_map = parse_bool(_value);
    else if (_key == "use_emissive_map")  _desc.use_emissive_map = parse_bool(_value);

    // material properties
    else if (_key == "albedo_color")   _desc.albedo_color = parse_vec4(_value);
    else if (_key == "metallic")       _desc.metallic = parse_float(_value);
    else if (_key == "roughness")      _desc.roughness = parse_float(_value);
    else if (_key == "ao")             _desc.ao = parse_float(_value);
    else if (_key == "tiling_factor")  _desc.tiling_factor = parse_float(_value);

    else {
        SYN_WARNING("material '%s': unknown property '%s'.\n", _mat_name.c_str(), _key.c_str());
    }
}

//
void asset_manager_t::apply_smart_defaults(material_descriptor_t &_desc)
{
    if (!_desc.albedo_texture.empty()       && !_desc.use_albedo_map)       _desc.use_albedo_map = 1.0f;
    if (!_desc.normal_texture.empty()       && !_desc.use_normal_map)       _desc.use_normal_map = 1.0f;
    if (!_desc.metallic_texture.empty()     && !_desc.use_metallic_map)     _desc.use_metallic_map = 1.0f;
    if (!_desc.roughness_texture.empty()    && !_desc.use_roughness_map)    _desc.use_roughness_map = 1.0f;
    if (!_desc.ao_texture.empty()           && !_desc.use_ao_map)           _desc.use_ao_map = 1.0f;
    if (!_desc.emissive_texture.empty()     && !_desc.use_emissive_map)     _desc.use_emissive_map = 1.0f;

}

//
void asset_manager_t::create_material_from_descriptor(const std::string &_mat_name, material_descriptor_t &_desc)
{
    auto shader_it = m_shader_map.find(_desc.shader_name);
    if (shader_it == m_shader_map.end()) {
        SYN_ERROR("material '%s' references unknown shader '%s'.\n", _mat_name.c_str(), _desc.shader_name.c_str());
        return;
    }

    // create the material
    material_handle_t mat_handle = mat_lib.create_material(shader_it->second);
    material_internal_t *mat = mat_lib.get_material(mat_handle);

    if (!mat) {
        SYN_ERROR("failed to create material '%s'.\n", _mat_name.c_str());
        return;
    }

    // assign textures
    assign_texture_slot(mat, 0, _desc.albedo_texture);
    assign_texture_slot(mat, 1, _desc.normal_texture);
    assign_texture_slot(mat, 2, _desc.metallic_texture);
    assign_texture_slot(mat, 3, _desc.roughness_texture);
    assign_texture_slot(mat, 4, _desc.ao_texture);
    assign_texture_slot(mat, 5, _desc.emissive_texture);

    material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;

    // flags
    pbr->use_albedo_map     = _desc.use_albedo_map      ? 1.0f : 0.0f;
    pbr->use_normal_map     = _desc.use_normal_map      ? 1.0f : 0.0f;
    pbr->use_metallic_map   = _desc.use_metallic_map    ? 1.0f : 0.0f;
    pbr->use_roughness_map  = _desc.use_roughness_map   ? 1.0f : 0.0f;
    pbr->use_ao_map         = _desc.use_ao_map          ? 1.0f : 0.0f;
    pbr->use_emissive_map   = _desc.use_emissive_map    ? 1.0f : 0.0f;

    // material values
    pbr->albedo_color   = _desc.albedo_color;
    pbr->metallic       = _desc.metallic;
    pbr->roughness      = _desc.roughness;
    pbr->ao             = _desc.ao;
    pbr->tiling_factor  = _desc.tiling_factor;

    m_material_map[_mat_name] = mat_handle;

    SYN_INFO("created material '%s' with shader '%s'.\n", _mat_name.c_str(), _desc.shader_name.c_str());

}

//
void asset_manager_t::assign_texture_slot(material_internal_t *_mat, size_t _slot, const std::string &_tex_name)
{
    if (_tex_name.empty()) {
        _mat->textures[_slot] = { 0 };
        return;
    }

    auto tex_it = m_texture_map.find(_tex_name);
    if (tex_it != m_texture_map.end()) {
        _mat->textures[_slot] = tex_it->second;
    } else {
        SYN_WARNING("texture '%s' not found.\n", _tex_name.c_str());
        _mat->textures[_slot] = { 0 };
    }
}

//
void asset_manager_t::parse_skybox(const std::vector<std::string> &_tokens)
{
    // format: skybox <name> <+x> <-x> <+y> <-y> <+z> <-z>
    if (_tokens.size() < 8) {
        SYN_ERROR("Invalid skybox entry: expected 'skybox <name> <+x> <-x> <+y> <-y> <+z> <-z>'.\n");
        return;
    }

    const std::string& name = _tokens[1];
    std::vector<std::string> faces = {
        _tokens[2], // +x
        _tokens[3], // -x
        _tokens[4], // +y
        _tokens[5], // -y
        _tokens[6], // +z
        _tokens[7]  // -z
    };

    update_load_progress("cubemap", name);

    cubemap_handle_t handle = cubemap_lib.load_cubemap(faces);
    m_cubemap_map[name] = handle;

    renderer.set_skybox(handle);

    SYN_INFO("loaded skybox '%s'.\n", name.c_str());
}

//
void asset_manager_t::parse_entity(const std::string &_block, const std::string &_entity_name)
{
    entity_descriptor_t desc;

    std::istringstream block_stream(_block);
    std::string line;

    update_load_progress("entity", _entity_name);

    // Skip first line (entity name {)
    std::getline(block_stream, line);

    while (std::getline(block_stream, line)) {
        //
        line = trim(line);

        if (line.empty() || line[0] == '#' || line[0] == '{' || line[0] == '}') continue;

        // Parse "key: value"
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            SYN_WARNING("invalid material property line (colon missing): '%s'.\n", line.c_str());
            continue;
        }


        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        if (!value.empty() && value.back() == ',') {
            value.pop_back();
        }

        parse_entity_property(desc, key, value, _entity_name);
    }

    create_entity_from_descriptor(_entity_name, desc);
}

//
void asset_manager_t::parse_entity_property(entity_descriptor_t &_desc,
                                            const std::string &_key,
                                            const std::string &_value,
                                            const std::string &_entity_name)
{
    if (_key == "mesh")           _desc.mesh_name = _value;
    else if (_key == "material")  _desc.material_name = _value;
    else if (_key == "position")  _desc.position = parse_vec3(_value);
    else if (_key == "rotation")  _desc.rotation_degrees = parse_vec3(_value);
    else if (_key == "scale")     _desc.scale = parse_vec3(_value);
    else {
        SYN_WARNING("entity '%s': unknown property '%s'.\n", _entity_name.c_str(), _key.c_str());
    }
}

//
void asset_manager_t::create_entity_from_descriptor(const std::string &_name,
                                                    const entity_descriptor_t &_desc)
{
    entity_t entity;
    entity.name = _name;

    // resolve mesh
    auto mesh_it = m_mesh_map.find(_desc.mesh_name);
    if (mesh_it != m_mesh_map.end()) {
        entity.mesh_handle = mesh_it->second;
        m_entity_mesh_map[_name] = entity.mesh_handle;
    } else {
        SYN_ERROR("entity '%s' references unknown mesh '%s'.\n", _name.c_str(), _desc.mesh_name.c_str());
        return;
    }

    // resolve material
    auto mat_it = m_material_map.find(_desc.material_name);
    if (mat_it != m_material_map.end()) {
        entity.material_handle = mat_it->second;
    } else {
        SYN_ERROR("entity '%s' references unknown material '%s'.\n", _name.c_str(), _desc.material_name.c_str());
        return;
    }

    // build transform
    entity.transform = entity_t::make_transform(_desc.position, _desc.rotation_degrees, _desc.scale);

    entity_handle_t handle = entity_lib.add_entity(entity);

    m_entity_map[_name] = handle;

    SYN_INFO("created entity '%s' (mesh='%s', material='%s')\n",
             _name.c_str(), _desc.mesh_name.c_str(), _desc.material_name.c_str());
}

//
bool asset_manager_t::parse_bool(const std::string &_value)
{
    if (_value == "true"  || _value == "TRUE"  || _value == "1" || _value == "yes") return true;
    if (_value == "false" || _value == "FALSE" || _value == "0" || _value == "no")  return false;

    SYN_WARNING("invalid boolean value '%s', defaulting to false.\n", _value.c_str());
    return false;

}

//
float asset_manager_t::parse_float(const std::string &_value)
{
    try {
        return std::stof(_value);
    } catch (...) {
        SYN_WARNING("invalid float value '%s', defaulting to 0.0f.\n", _value.c_str());
        return 0.0f;
    }

}

//
glm::vec3 asset_manager_t::parse_vec3(const std::string &_value)
{
    std::istringstream iss(_value);
    float x, y, z;

    if (iss >> x >> y >> z) {
        return glm::vec3(x, y, z);
    }
    else if (iss.clear(), iss.seekg(0), iss >> x) {
        return glm::vec3(x);
    }

    SYN_WARNING("invalid vec3 value '%s', defaulting to vec3(1.0f).\n", _value.c_str());
    return glm::vec3(1.0f);

}

//
glm::vec4 asset_manager_t::parse_vec4(const std::string &_value)
{
    std::istringstream iss(_value);
    float x, y, z, w;

    if (iss >> x >> y >> z >> w) {
        return glm::vec4(x, y, z, w);
    }
    else if (iss.clear(), iss.seekg(0), iss >> x) {
        return glm::vec4(x);
    }

    SYN_WARNING("invalid vec3 value '%s', defaulting to vec3(1.0f).\n", _value.c_str());
    return glm::vec4(1.0f);

}

//
void asset_manager_t::count_assets_in_manifest(const std::vector<std::string> &_lines)
{
    m_load_progress.total_assets = 0;
    for (const auto &line : _lines) {
        std::string first_line = line.substr(0, line.find('\n'));
        std::istringstream iss(first_line);
        std::string type;
        iss >> type;

        if (type == "shader" || type == "texture" || type == "mesh" ||
            type == "material" || type == "skybox" || type == "entity") {
            m_load_progress.total_assets++;
        }
    }

    SYN_INFO("total assets to load: %d\n", m_load_progress.total_assets);

    // add one extra to make it 100% at fully loaded, not when loading the last asset
    m_load_progress.total_assets++;

}

//
void asset_manager_t::update_load_progress(const std::string _type, const std::string &_name)
{
    m_load_progress.current_asset = _type + ": " + _name;
    m_load_progress.loaded_assets++;

    if (m_load_progress.enabled) {
        render_loading_assets();
    }

}

//
void asset_manager_t::render_loading_assets()
{
    api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float progress = 0.0f;
    if (m_load_progress.total_assets > 0) {
        progress = (float)m_load_progress.loaded_assets / (float)m_load_progress.total_assets;
    }

    glm::ivec2 dims = root_window.get_window_dims();
    api.set_viewport({ 0, 0 }, dims);

    float bar_width = dims.x * 0.5f;
    float bar_height = dims.y * 0.02f;
    float bar_x = (dims.x - bar_width) / 2.0f;
    float bar_y = (dims.y + bar_height) / 2.0f;

    glm::vec2 pos  = { bar_x, bar_y };
    glm::vec2 size = { bar_width, bar_height };
    glm::vec4 lc = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    renderer_2d.batch.add_quad(pos, size, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), -1.0f);
    ui_render_vertex_t ls[] = {
        ui_render_vertex_t({ pos.x,          pos.y          }, lc, 0.0f),
        ui_render_vertex_t({ pos.x + size.x, pos.y          }, lc, 0.0f),
        ui_render_vertex_t({ pos.x + size.x, pos.y + size.y }, lc, 0.0f),
        ui_render_vertex_t({ pos.x,          pos.y + size.y }, lc, 0.0f),
        ui_render_vertex_t({ pos.x,          pos.y          }, lc, 0.0f),
    };
    renderer_2d.batch.add_line_strip(ls, 5);
    
    float filled_width = bar_width * progress;
    renderer_2d.batch.add_quad(pos, { filled_width, bar_height }, glm::vec4(0.65f, 0.30f, 0.04f, 1.0f), -0.5f);

    //    
    int percent = (int)(progress * 100.0f);
    float percent_x = bar_x + bar_width * 0.5f - font.get_string_width("%d%%", percent) * 0.5f;
    float asset_text_y = bar_y - font.get_font_glyph_height() * 0.5f;

    float percent_y = bar_y + (bar_height + font.get_font_glyph_height()) * 0.5f;

    renderer_2d.batch.end_batch();

    font.render_text(percent_x, percent_y, "%d%%", percent);
    font.render_text(bar_x, asset_text_y, "Loading assets... %s", m_load_progress.current_asset.c_str());
    font.end_render_block(false);

    root_window.post_render();
    glfwPollEvents();


}

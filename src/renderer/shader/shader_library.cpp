
#include "renderer/shader/shader_library.h"

#include "core.h"
#include "utils/log.h"
#include "utils/file_io_handler.h"
#include "utils/scope_timer.h"

#include "c_api.h"

// 
void shader_library_t::shutdown()
{
    SYN_INFO("deleting shaders...\n");
    
    for (size_t i = 0; i < m_shader_count; ++i) {
        SYN_INFO("%s\n", m_shaders[i].get_name().c_str());
        m_shaders[i].destroy();
    }
    m_shader_count = 0;
    
    SYN_INFO("clearing shader handles...\n");
    m_name_to_handle_map.clear();
}

// 
shader_handle_t shader_library_t::load_from_file(const std::string &_name, 
                                                 const std::string &_shader_file_path)
{
    // already present?
    auto it = m_name_to_handle_map.find(_name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }

    std::string src;
    if (file_io_handler.read_file_to_buffer(_shader_file_path, src) != RETURN_SUCCESS) {
        SYN_WARNING("could not read shader file '%s'\n", _shader_file_path.c_str());
        return (shader_handle_t){ 0 };
    }

    m_last_loaded_asset_path = _shader_file_path;
    
    return load_shader_source(_name, src);
    
}

// 
shader_handle_t shader_library_t::load_from_source(const std::string &_name, 
                                                   const std::string &_shader_src)
{
    // already present?
    auto it = m_name_to_handle_map.find(_name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }

    if (m_shader_count >= SYN_MAX_SHADERS) {
        SYN_WARNING("shader_count = SYN_MAX_SHADERS.\n");
        return shader_handle_t { 0 };
    }

    m_last_loaded_asset_path = "";
    
    return load_shader_source(_name, _shader_src);
    
}

// 
shader_handle_t shader_library_t::load_shader_source(const std::string &_name, const std::string &_shader_src)
{
    // index + 1 is used as ID; ID=0 represents and invalid shader
    uint32_t slot = (uint32_t)m_shader_count;
    shader_handle_t handle { slot + 1 };

    // shader initalized directly in the array, no pointers used
    m_shaders[slot] = shader_t();
    m_shaders[slot].load_from_source(_name, _shader_src);
    m_shaders[slot].set_asset_path(m_last_loaded_asset_path);
    
    //
    m_name_to_handle_map[_name] = handle;
    m_shader_count++;
    
    #ifdef DEBUG_SHADER_LIBRARY
    SYN_INFO("loaded shader '%s' [%d].\n", _name.c_str(), handle.id);
    #endif
    
    return handle;
    
}

// 
void shader_library_t::reload_shaders()
{
    scope_timer_t timer;
    
    uint32_t success = 0;
    uint32_t failed = 0;
    uint32_t checked = 0;
    uint32_t changed = 0;

    std::vector<GLuint> old_ids;
    
    // 
    for (uint32_t i = 0; i < m_shader_count; i++) {
        shader_t *shader = &m_shaders[i];
        if (!shader->is_active()) continue;

        std::string path = shader->get_asset_path();
        if (path.empty()) continue;

        checked++;

        // has file changed?
        if (!shader->file_has_changed()) continue;

        changed++;

        std::string name = shader->get_name();
        GLuint old_id = shader->get_id();
        SYN_INFO("reloading '%s' [%s].\n", name.c_str(), path.c_str());
        
        shader->reload();

        if (shader->get_id() == 0) {
            SYN_WARNING("failed to reload shader '%s' [%s].\n", name.c_str(), path.c_str());
            failed++;
        } else {
            old_ids.push_back(old_id);
            success++;
        }

    }

    for (auto &old_id : old_ids) {
        glDeleteProgram(old_id);
    }

    renderer.show_notification("INFO: Shaders reloaded");

    SYN_INFO("shader reload: %d checked, %d changed, %d reloaded, %d failed (%.2f ms).\n", 
             checked, changed, success, failed, timer.get_dt_ms());
}

// 
shader_handle_t shader_library_t::get_handle(const std::string& name)
{
    auto it = m_name_to_handle_map.find(name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }
    return shader_handle_t{ 0 };
}

// 
shader_t *shader_library_t::get_shader(shader_handle_t handle)
{
    // check index in bounds
    uint32_t index = handle.id - 1;
    if (handle.id == 0 || index >= m_shader_count) {
        return nullptr;
    }
    
    // pointer to the array shader
    return &m_shaders[index];
}

// 
shader_t *shader_library_t::get_by_opengl_id(uint32_t _opengl_id)
{
    for (uint32_t i = 0; i < m_shader_count; i++) {
        shader_t &shader = m_shaders[i];
        if (shader.get_id() == _opengl_id) {
            return &shader;
        }
    }
    
    return nullptr;
}
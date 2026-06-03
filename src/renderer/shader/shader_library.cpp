
#include "renderer/shader/shader_library.h"

#include "core.h"
#include "utils/log.h"
#include "utils/file_io_handler.h"


// 
void shader_library_t::shutdown()
{
    SYN_INFO("deleting shaders...\n");
    
    for (size_t i = 0; i < m_shader_count; ++i) {
        SYN_INFO("%s\n", m_shaders[i].m_shader_name.c_str());
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
    
    return load_from_source(_name, src);
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

    // index + 1 is used as ID; ID=0 represents and invalid shader
    uint32_t slot = (uint32_t)m_shader_count;
    shader_handle_t handle { slot + 1 };

    // shader initalized directly in the array, no pointers used
    m_shaders[slot] = shader_t();
    m_shaders[slot].load_from_source(_name, _shader_src);

    //
    m_name_to_handle_map[_name] = handle;
    m_shader_count++;
    
    #ifdef DEBUG_SHADER_LIBRARY
    SYN_INFO("loaded shader '%s' [%d].\n", _name.c_str(), handle.id);
    #endif
    
    return handle;
    
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
shader_t *shader_library_t::get(shader_handle_t handle)
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
        if (shader.m_opengl_id == _opengl_id) {
            return &shader;
        }
    }
    
    return nullptr;
}
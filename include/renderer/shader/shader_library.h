#ifndef __SHADER_LIBRARY_EX_H
#define __SHADER_LIBRARY_EX_H

#include <stdint.h>
#include <unordered_map>
#include <string>

#include "renderer/shader/shader_types.h"
#include "renderer/shader/shader.h"

// 
#define SYN_MAX_SHADERS 64

//
class shader_library_t
{
public:
    shader_library_t() = default;
    ~shader_library_t() = default;

    void shutdown();

    // loads and compiles a shader, returns handle
    shader_handle_t load_from_file(const std::string &_name, const std::string &_shader_file_path);
    shader_handle_t load_from_source(const std::string &_name, const std::string &_shader_src);
private:
    shader_handle_t load_shader_source(const std::string &_name, const std::string &_shader_src);

public:
    void reload_shaders();
    
    //
    shader_handle_t get_handle(const std::string &_name);
    shader_t *get(shader_handle_t handle);
    shader_t *get_by_opengl_id(uint32_t _opengl_id);


private:
    shader_t m_shaders[SYN_MAX_SHADERS];
    size_t m_shader_count = 0;

    std::string m_last_loaded_asset_path = "";
    
    std::unordered_map<std::string, shader_handle_t> m_name_to_handle_map;
    
};




#endif // __SHADER_LIBRARY_EX_H


#include "external/glad/glad.h"
#include "external/stb/stb_image.h"

#include "renderer/material/cubemap_library.h"
#include "utils/log.h"


// 
void cubemap_library_t::init()
{
    memset(m_pool, 0, sizeof(cubemap_internal_t) * SYN_MAX_CUBEMAPS);
    m_active_count = 0;
    
}

// 
void cubemap_library_t::shutdown()
{
    for (size_t i = 0; i < SYN_MAX_CUBEMAPS; i++) {
        if (m_pool[i].is_active) {
            glDeleteTextures(1, &m_pool[i].opengl_id);
        }
    }
}

//
cubemap_handle_t cubemap_library_t::load_cubemap(const std::vector<std::string> _face_filepaths)
{
    if (_face_filepaths.size() != 6) {
        SYN_WARNING("six faces needed for cubemap generation.\n");
        return { 0 };
    }

    uint32_t slot = m_active_count;
    if (slot >= SYN_MAX_CUBEMAPS) {
        SYN_WARNING("max cubemaps already used (SYN_MAX_CUBEMAPS).\n");
        return { 0 };
    }
    
    cubemap_internal_t &cubemap = m_pool[slot];
    m_active_count++;

    glGenTextures(1, &cubemap.opengl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.opengl_id);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);

    for (size_t i = 0; i < 6; i++) {
        uint8_t *data = stbi_load(_face_filepaths[i].c_str(), &width, &height, &channels, 3);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            SYN_WARNING("cubemap face failed to load (%s).\n", _face_filepaths[i].c_str());
            return { 0 };
        }
        
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    cubemap.width = width;
    cubemap.height = height;
    cubemap.is_active = true;

    return { slot + 1 };
    
}

// 
cubemap_internal_t *cubemap_library_t::get_cubemap(cubemap_handle_t _handle)
{
    uint32_t index = _handle.id - 1;
    if (!_handle.is_valid() || index >= m_active_count) {
        return nullptr;
    }
    return &m_pool[index];
}

// 
cubemap_handle_t cubemap_library_t::create_empty(uint32_t _width, uint32_t _height, uint32_t _internal_format, bool _mipmap)
{
    uint32_t slot = m_active_count;
    if (slot >= SYN_MAX_CUBEMAPS) {
        SYN_WARNING("max cubemaps already used (SYN_MAX_CUBEMAPS).\n");
        return { 0 };
    }
    
    cubemap_internal_t &cubemap = m_pool[slot];
    m_active_count++;

    glGenTextures(1, &cubemap.opengl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.opengl_id);

    // determine format and type
    GLenum format = (_internal_format == GL_RGBA16F || _internal_format == GL_RGBA) ? GL_RGBA : GL_RGB;
    GLenum type = (_internal_format == GL_RGBA16F || _internal_format == GL_RGB16F) ? GL_FLOAT : GL_UNSIGNED_BYTE;

    for (size_t i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, _internal_format, _width, _height, 0, format, type, NULL);        
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    if (_mipmap) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    } else {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    cubemap.width = _width;
    cubemap.height = _height;
    cubemap.is_active = true;

    return { slot + 1 };
    
}
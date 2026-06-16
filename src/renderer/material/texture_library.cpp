
#include <string.h>
#ifndef GLAD_INCLUDED
#include "external/glad/glad.h"
#endif

#include "external/stb/stb_image.h"

#include "renderer/material/texture_library.h"
#include "utils/log.h"

// 
void texture_library_t::init()
{
    memset(m_pool, 0, sizeof(texture_internal_t) * SYN_MAX_TEXTURE_COUNT);
    m_active_count = 0;
    
}

// 
void texture_library_t::shutdown()
{
    for (uint32_t i = 0; i < SYN_MAX_TEXTURE_COUNT; i++) {
        if (m_pool[i].is_active && m_pool[i].opengl_id != 0) {
            glDeleteTextures(1, &m_pool[i].opengl_id);
        }
    }
}

// 
texture_handle_t texture_library_t::load_texture(const std::string &_filepath)
{
    // search for duplicates
    for (uint32_t i = 1; i < SYN_MAX_TEXTURE_COUNT; i++) {
        if (m_pool[i].is_active && m_pool[i].asset_path == _filepath) {
            return { i };
        }
    }

    // find empty memory (in case release_texture() was called)
    // TODO : this could be sped up using a flag later, e.g. 
    //        m_no_textures_deleted = true    
    uint32_t free_slot = 0;
    for (uint32_t i = 1; i < SYN_MAX_TEXTURE_COUNT; i++) {
        if (!m_pool[i].is_active) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == 0) {
        SYN_WARNING("SYN_MAX_TEXTURE_COUNT reached. Texture from '%s' not loaded.\n", _filepath.c_str());
        return { 0 };
    }

    // load into memory (stbi)
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(_filepath.c_str(), &width, &height, &channels, 4);

    if (!data) {
        SYN_WARNING("failed to load texture from '%s'.\n", _filepath.c_str());
        return { 0 };
    }

    // register internal struct specs
    texture_internal_t &texture = m_pool[free_slot];
    texture.width      = (uint32_t)width;
    texture.height     = (uint32_t)height;
    texture.channels   = 4;
    texture.asset_path = _filepath;
    
    // generate opengl texture
    glCreateTextures(GL_TEXTURE_2D, 1, &texture.opengl_id);
    glBindTexture(GL_TEXTURE_2D, texture.opengl_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // free image data
    stbi_image_free(data);

    texture.is_active = true;
    m_active_count++;

    return { free_slot };
    
}

// 
texture_handle_t texture_library_t::load_texture_hdr(const std::string &_filepath)
{
    // search for duplicates
    for (uint32_t i = 1; i < SYN_MAX_TEXTURE_COUNT; i++) {
        if (m_pool[i].is_active && m_pool[i].asset_path == _filepath) {
            return { i };
        }
    }

    // find empty memory (in case release_texture() was called)
    // TODO : this could be sped up using a flag later, e.g. 
    //        m_no_textures_deleted = true    
    uint32_t free_slot = 0;
    for (uint32_t i = 1; i < SYN_MAX_TEXTURE_COUNT; i++) {
        if (!m_pool[i].is_active) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == 0) {
        SYN_WARNING("SYN_MAX_TEXTURE_COUNT reached. Texture from '%s' not loaded.\n", _filepath.c_str());
        return { 0 };
    }

    // load HDR texture using stbi:s float version
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf(_filepath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        SYN_WARNING("failed to load HDR texture from '%s'.\n", _filepath.c_str());
        return { 0 };
    }

    texture_internal_t &tex = m_pool[free_slot];
    tex.width      = width;
    tex.height     = height;
    tex.channels   = channels;
    tex.asset_path = _filepath;

    // create opengl texture
    // generate opengl texture
    glCreateTextures(GL_TEXTURE_2D, 1, &tex.opengl_id);
    glBindTexture(GL_TEXTURE_2D, tex.opengl_id);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    tex.is_active = true;
    m_active_count++;

    SYN_INFO("loaded HDR texture %d (%d x %d) from '%s'.\n", free_slot, width, height, _filepath.c_str());

    return { free_slot };

}

// 
texture_internal_t *texture_library_t::get_texture(texture_handle_t _handle)
{
    if (_handle.id == 0 || _handle.id >= SYN_MAX_TEXTURE_COUNT || !m_pool[_handle.id].is_active) {
        return nullptr;
    }

    return &m_pool[_handle.id];
}

// 
void texture_library_t::release_texture(texture_handle_t _handle)
{
    if (_handle.id > 0 && _handle.id < SYN_MAX_TEXTURE_COUNT) {
        texture_internal_t &texture = m_pool[_handle.id];
        if (texture.is_active) {
            glDeleteTextures(1, &texture.opengl_id);
            memset(&texture, 0, sizeof(texture_internal_t));
            m_active_count--;
        }
    }
}

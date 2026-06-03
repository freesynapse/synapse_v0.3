#ifndef __TEXTURE_LIBRARY_H
#define __TEXTURE_LIBRARY_H

#include <string>

#include "renderer/material/texture_types.h"

// 
class texture_library_t
{
public:
    void init();
    void shutdown();

    texture_handle_t load_texture(const std::string &_filepath);
    texture_handle_t load_texture_hdr(const std::string &_filepath);
    
    texture_internal_t *get_texture(texture_handle_t _handle);
    void release_texture(texture_handle_t _handle);

    texture_internal_t m_pool[SYN_MAX_TEXTURE_COUNT];
    uint32_t m_active_count;
    
};

#endif // __TEXTURE_LIBRARY_H

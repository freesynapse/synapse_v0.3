#ifndef __CUBEMAP_LIBRARY_H
#define __CUBEMAP_LIBRARY_H

#include <vector>
#include <string>

#include "renderer/material/cubemap_types.h"

// 
#define SYN_MAX_CUBEMAPS    8

// 
class cubemap_library_t
{
public:
    void init();
    void shutdown();

    // expects faces in order: +x, -x, +y, -y, +z, -z
    cubemap_handle_t load_cubemap(const std::vector<std::string> _face_filepaths);
    cubemap_internal_t *get_cubemap(cubemap_handle_t _handle);

    cubemap_handle_t create_empty(uint32_t _width, uint32_t _height, uint32_t _internal_format, bool _mipmap=false);

private:
    cubemap_internal_t m_pool[SYN_MAX_CUBEMAPS];
    uint32_t m_active_count = 0;
    
};


#endif // __CUBEMAP_LIBRARY_H


#include "renderer/buffers/frambuffer_handler.h"
#include "utils/log.h"

// 
void framebuffer_handler_t::init()
{
    
}

// 
void framebuffer_handler_t::shutdown()
{
    for (uint32_t i = 0; i < m_framebuffer_count; i++)
    {
        glDeleteFramebuffers(1, &m_framebuffers[i].m_framebuffer_id);
    }
}


// 
framebuffer_handle_t framebuffer_handler_t::create_framebuffer(
                const color_format_t &_format, 
                const glm::ivec2 &_size,
                size_t _n_drawbuffers,
                bool _use_depthbuffer,
                const std::string &_name)
{
    if (m_framebuffer_count >= SYN_MAX_FRAMEBUFFERS) {
        SYN_WARNING("max framebuffers reached.\n");
        return { 0 };
    }
    
    uint32_t slot = m_framebuffer_count;
    m_framebuffers[slot].create(_format, _size, _n_drawbuffers, _use_depthbuffer, _name);
    m_framebuffer_count++;

    return { slot + 1 };
    
}

//
framebuffer_t *framebuffer_handler_t::get_framebuffer(const framebuffer_handle_t &_handle) 
{
    if (!_handle.is_active() || _handle.id > m_framebuffer_count) {
        return nullptr;
    }
    
    return &m_framebuffers[_handle.id - 1];
    
}

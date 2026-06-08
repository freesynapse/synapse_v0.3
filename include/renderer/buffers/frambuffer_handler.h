#ifndef __FRAMEBUFFER_HANDLER_H
#define __FRAMEBUFFER_HANDLER_H

#include "renderer/buffers/framebuffer.h"
#include "renderer/buffers/framebuffer_types.h"

// 
#define SYN_MAX_FRAMEBUFFERS    16

// 
class framebuffer_handler_t
{
public:
    framebuffer_handler_t() = default;
    ~framebuffer_handler_t() = default;
    
    void init();
    void shutdown();

    // TODO : wierd to have this here, move or delete?
    void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    
    framebuffer_handle_t create_framebuffer(const color_format_t &_format, 
                                            const glm::ivec2 &_size,
                                            size_t _n_drawbuffers,
                                            bool _use_depthbuffer,
                                            const std::string &_name);

    framebuffer_t *get_framebuffer(const framebuffer_handle_t &_handle);

private:
    framebuffer_t m_framebuffers[SYN_MAX_FRAMEBUFFERS];
	uint32_t m_framebuffer_count = 0;

};


#endif // __FRAMEBUFFER_HANDLER_H

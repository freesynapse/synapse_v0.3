#ifndef __RENDERER2D_H
#define __RENDERER2D_H

#include <glm/glm.hpp>
#include <array>

#include "renderer/renderer_types.h"
#include "renderer/buffers/vertex_array.h"
#include "renderer/shader/shader_types.h"
#include "renderer/UI/ui_render_batch.h"

// 
class renderer_2d_t
{
public:
    renderer_2d_t() = default;
    ~renderer_2d_t() = default;
    
    void init();
    void shutdown();

    // batch renderer for ui elements
    ui_render_batch_t batch;
    
    
    // conversion of of quad_t to vertex_2d_t[4] for batch rendering
private:
     std::array<vertex_data_2d_t, 4> quad_t_to_vertex_2d_t(const quad_2d_t &_q);

    void init_ui_quad();
    void init_window_quad();

private:
    // quad drawing
	vertex_array_t m_ui_quad_vao;
	shader_handle_t m_ui_shader_handle;
	
    
};


#endif // __RENDERER2D_H

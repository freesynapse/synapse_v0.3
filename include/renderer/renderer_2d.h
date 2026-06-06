#ifndef __RENDERER2D_H
#define __RENDERER2D_H

#include <glm/glm.hpp>

#include "renderer/buffers/vertex_array.h"
#include "renderer/shader/shader_types.h"


// 
class renderer_2d_t
{
public:
    renderer_2d_t() = default;
    ~renderer_2d_t() = default;
    
    void init();
    void shutdown();

    void init_ui_quad();
	void draw_rect(float _x, float _y, float _w, float _h, const glm::vec4 &_color);
	void draw_rect_outline(float _x, float _y, 
	                       float _w, float _h, 
	                       float _thickness, 
	                       const glm::vec4 &_color, 
	                       const glm::vec4 &_outline_color);
   
private:
    // quad drawing
	vertex_array_t m_ui_quad_vao;
	shader_handle_t m_ui_shader_handle;
	
    
};


#endif // __RENDERER2D_H

#ifndef __GLAPI_H
#define __GLAPI_H

#include <string>

#ifndef GLAD_INCLUDED
#include "external/glad/glad.h"
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "event/event.h"


//
class gl_api_t
{
public:
    friend class renderer_t;
    friend class renderer_2d_t;
    
public:
    gl_api_t() = default;
    ~gl_api_t() = default;

    void init();
    void on_resize(const event_t &_e);
    
    // API wrappers
    void clear_color_buffer();
    void clear_depth_buffer();
    void clear(uint32_t _bitfield);
    
    void set_blending_eq(GLenum _src_factor, GLenum _dest_factor);
    
    void set_wireframe(bool _wireframe);
    void set_depth_testing(bool _depth_test);
    void set_depth_func(GLenum _func);
    void set_depth_mask(bool _depth_mask);
    void set_culling(bool _cull);
    void set_blending(bool _blending);
    void set_GLenum(GLenum _gl_enum, bool _b);
    
    void set_line_width(float _width);

	void set_clear_color(float _r, float _g, float _b, float _a);
	const glm::vec4 &get_clear_color() { return m_clear_color; }
    void set_clear_color(const glm::vec4 &_color);

    const glm::ivec2 &get_viewport();
    const glm::vec2 get_viewport_f();
	float get_aspect_ratio();      
    void set_viewport(const glm::ivec2 &_position, const glm::ivec2 &_size);
    void reset_viewport();
    
    // error checking
    std::string &get_gl_error_string(GLenum _error_code);
    int gl_error(const char* _calling_func, const char* _gl_call="");
    void GLAPIENTRY openGLLogMessage(GLenum _src, 
								 GLenum _type, 
								 GLuint _id, 
								 GLenum _severity, 
								 GLsizei _len, 
								 const GLchar *_msg, 
								 const void *_params);
    

private:
    // renderer variables
	glm::ivec2 m_viewport = { 0, 0 };
	glm::vec4 m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	bool m_is_blending = true;

	// 
    std::string m_last_gl_error;
    
};




#endif // __GLAPI_H
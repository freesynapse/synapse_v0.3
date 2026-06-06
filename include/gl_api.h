#ifndef __GLAPI_H
#define __GLAPI_H

#ifndef GLAD_INCLUDED
#include "external/glad/glad.h"
#endif

#include <GLFW/glfw3.h>

#include <string>

//

class gl_api_t
{
public:
    gl_api_t() = default;
    ~gl_api_t() = default;
    
    // API wrappers
    void clear_color_buffer();
    void clear_depth_buffer();
    void clear(uint32_t _bitfield);
    
    void set_blending_eq(GLenum _src_factor, GLenum _dest_factor);
    
    void set_wireframe(bool _wireframe);
    void set_depth_testing(bool _depth_test);
    void set_depth_mask(bool _depth_mask);
    void set_culling(bool _cull);
    void set_blending(bool _blending);
    void set_GLenum(GLenum _gl_enum, bool _b);
    
    void set_line_width(float _width);

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
    std::string m_last_gl_error;
    
};




#endif // __GLAPI_H
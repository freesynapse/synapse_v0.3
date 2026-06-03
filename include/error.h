#ifndef __ERROR_H
#define __ERROR_H

#include <string>

#ifndef GLAD_INCLUDED
#include "glapi.h"
#endif

#include "core.h"
#include "utils/log.h"

//
static std::string s_last_gl_error = "GL_NO_ERROR";

//
static std::string &get_gl_error_string(GLenum _error_code)
{
    switch (_error_code)
    {
        case GL_INVALID_ENUM: 					s_last_gl_error = "GL_INVALID_ENUM"; 				    break;
        case GL_INVALID_VALUE: 					s_last_gl_error = "GL_INVALID_VALUE"; 				    break;
        case GL_INVALID_OPERATION: 				s_last_gl_error = "GL_INVALID_OPERATION"; 			    break;
        case GL_STACK_OVERFLOW: 				s_last_gl_error = "GL_STACK_OVERFLOW"; 				    break;
        case GL_STACK_UNDERFLOW: 				s_last_gl_error = "GL_STACK_UNDERFLOW"; 			    break;
        case GL_OUT_OF_MEMORY: 					s_last_gl_error = "GL_OUT_OF_MEMORY"; 				    break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: 	s_last_gl_error = "GL_INVALID_FRAMEBUFFER_OPERATION";   break;
        default: s_last_gl_error = "(unknonw gl error code)"; break;
    }
    return s_last_gl_error;
}

//
static int gl_error(const char* _calling_func, const char* _gl_call="")
{
    GLenum error = GL_NO_ERROR;
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        SYN_ERROR("%s, %s: %s\n", _calling_func, _gl_call, get_gl_error_string(error).c_str());
        return RETURN_FAILURE;
    }

    return RETURN_SUCCESS;
}

static void GLAPIENTRY openGLLogMessage(GLenum _src, 
										GLenum _type, 
										GLuint _id, 
										GLenum _severity, 
										GLsizei _len, 
										const GLchar *_msg, 
										const void *_params)
{
	if (_severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		SYN_ERROR("%s\n", _msg);
		return;
	}
	SYN_INFO("%s\n", _msg);
}



#endif // __ERROR_H

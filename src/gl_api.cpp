
#include "gl_api.h"

#include "utils/log.h"

#include "c_api.h"


// static event callback wrappers
static void __gl_api_on_resize_callback(const event_t &_e) { api.on_resize(_e); }

//
void gl_api_t::init()
{
    m_viewport = root_window.window_dims();

    // 
    fbo_handler.init();
    
    // register function to receive viewport resize events
    events.register_callback(event_type_t::VIEWPORT_RESIZE, __gl_api_on_resize_callback);

}

//
void gl_api_t::on_resize(const event_t &_e)
{

    glm::ivec2 new_viewport = _e.as.viewport_resize.viewport;

    // set main viewport
    if (new_viewport.x > 0 && new_viewport.y > 0) {
        m_viewport = new_viewport;
        set_viewport(glm::ivec2(0, 0), new_viewport);

    } else {
        SYN_WARNING("viewport not set : new viewport = [%d, %d]\n", new_viewport.x, new_viewport.y);
    }
}

//
void gl_api_t::clear_color_buffer() { glClear(GL_COLOR_BUFFER_BIT); }
void gl_api_t::clear_depth_buffer() { glClear(GL_DEPTH_BUFFER_BIT); }
void gl_api_t::clear(uint32_t _bitfield) { glClear(_bitfield); }

//
void gl_api_t::set_blending_eq(GLenum _src_factor, GLenum _dest_factor)
{
    glBlendFunc(_src_factor, _dest_factor);
}

//
void gl_api_t::set_wireframe(bool _wireframe)
{
    if (_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        return;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//
void gl_api_t::set_depth_testing(bool _depth_test)
{
    if (_depth_test) {
        glEnable(GL_DEPTH_TEST);
        return;
    }
    glDisable(GL_DEPTH_TEST);
}

// 
void gl_api_t::enable_depth_test()
{
    glEnable(GL_DEPTH_TEST);
}

// 
void gl_api_t::disable_depth_test()
{
    glDisable(GL_DEPTH_TEST);
}

// 
void gl_api_t::set_depth_func(GLenum _func)
{
    glDepthFunc(_func);
}

//
void gl_api_t::set_depth_mask(bool _depth_mask)
{
    if (_depth_mask) {
        glDepthMask(GL_TRUE);
        return;
    }
    glDepthMask(GL_FALSE);
}

//
void gl_api_t::set_culling(bool _cull)
{
    if (_cull) {
        glEnable(GL_CULL_FACE);
        return;
    }
    glDisable(GL_CULL_FACE);
}

//
void gl_api_t::set_blending(bool _blending)
{
    if (_blending) {
        glEnable(GL_BLEND);
        return;
    }
    glDisable(GL_BLEND);
}

//
void gl_api_t::set_GLenum(GLenum _gl_enum, bool _b)
{
    if (_b) {
        glEnable(_gl_enum);
        return;
    }
    glDisable(_gl_enum);
}

//
void gl_api_t::set_line_width(float _width) { glLineWidth(_width); }

//
const glm::ivec2 &gl_api_t::get_viewport()
{
    return m_viewport;
}

//
const glm::vec2 gl_api_t::get_viewport_f()
{
    return glm::vec2(m_viewport.x, m_viewport.y);
}

//
float gl_api_t::get_aspect_ratio()
{
    return (float)m_viewport.x / (float)m_viewport.y;

}

void gl_api_t::set_viewport(const glm::ivec2 &_position, const glm::ivec2 &_size)
{
    glViewport(_position.x, _position.y, _size.x, _size.y);

}

//
void gl_api_t::reset_viewport()
{
    glViewport(0, 0, m_viewport.x, m_viewport.y);

}

//
void gl_api_t::set_clear_color(float _r, float _g, float _b, float _a)
{
    m_clear_color = glm::vec4(_r, _g, _b, _a);
    glClearColor(_r, _g, _b, _a);

}

//
void gl_api_t::set_clear_color(const glm::vec4 &_color)
{
    m_clear_color = _color;
    glClearColor(_color.r, _color.g, _color.b, _color.a);

}

//
std::string &gl_api_t::get_gl_error_string(GLenum _error_code)
{
    switch (_error_code)
    {
        case GL_NO_ERROR:                       m_last_gl_error = "GL_NO_ERROR";                        break;
        case GL_INVALID_ENUM: 					m_last_gl_error = "GL_INVALID_ENUM"; 				    break;
        case GL_INVALID_VALUE: 					m_last_gl_error = "GL_INVALID_VALUE"; 				    break;
        case GL_INVALID_OPERATION: 				m_last_gl_error = "GL_INVALID_OPERATION"; 			    break;
        case GL_STACK_OVERFLOW: 				m_last_gl_error = "GL_STACK_OVERFLOW"; 				    break;
        case GL_STACK_UNDERFLOW: 				m_last_gl_error = "GL_STACK_UNDERFLOW"; 			    break;
        case GL_OUT_OF_MEMORY: 					m_last_gl_error = "GL_OUT_OF_MEMORY"; 				    break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: 	m_last_gl_error = "GL_INVALID_FRAMEBUFFER_OPERATION";   break;
        default: m_last_gl_error = "(unknonw gl error code)"; break;
    }
    return m_last_gl_error;
}

//
int gl_api_t::gl_error(const char* _calling_func, const char* _gl_call)
{
    GLenum error = GL_NO_ERROR;
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        SYN_ERROR("%s, %s: %s\n", _calling_func, _gl_call, get_gl_error_string(error).c_str());
        return -1;
    }

    return 0;
}

//
void GLAPIENTRY gl_api_t::openGLLogMessage(GLenum _src,
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

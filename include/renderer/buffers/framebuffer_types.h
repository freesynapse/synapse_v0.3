#ifndef __FRAMEBUFFER_TYPES_H
#define __FRAMEBUFFER_TYPES_H

#include <stdint.h>
#include <stddef.h>

#include "external/glad/glad.h"


// 
enum class color_format_t : uint8_t {
	NONE	=  0,
	R8		=  1,
	RG8		=  2,
	RGB8 	=  3,
	RGBA8	=  4,
	R16F	=  5,
	RG16F	=  6,
	RGB16F	=  7,
	RGBA16F =  8,
	R32F	=  9,
	RG32F	= 10,
	RGB32F	= 11,
	RGBA32F	= 12
};

struct opengl_pixel_format_t
{
	GLint internalFormat;
	GLenum storageFormat;
	GLenum storageType;

	opengl_pixel_format_t() {}
	opengl_pixel_format_t(GLint _i, GLenum _s, GLenum _t) :
		internalFormat(_i), storageFormat(_s), storageType(_t) 
	{}

};


static inline opengl_pixel_format_t getOpenGLPixelFormat(const color_format_t& _fmt) {
	opengl_pixel_format_t fmt;

	switch (_fmt)
	{
		case color_format_t::NONE:		fmt = opengl_pixel_format_t(GL_NONE, 	GL_NONE, GL_NONE); 			break;
		case color_format_t::R8:		fmt = opengl_pixel_format_t(GL_R8,    	GL_RED,  GL_UNSIGNED_BYTE);	break;
		case color_format_t::RG8:		fmt = opengl_pixel_format_t(GL_RG8,   	GL_RG,   GL_UNSIGNED_BYTE);	break;
		case color_format_t::RGB8:		fmt = opengl_pixel_format_t(GL_RGB8,  	GL_RGB,  GL_UNSIGNED_BYTE);	break;
		case color_format_t::RGBA8:	    fmt = opengl_pixel_format_t(GL_RGBA8, 	GL_RGBA, GL_UNSIGNED_BYTE);	break;
		case color_format_t::R16F:		fmt = opengl_pixel_format_t(GL_R16F,    GL_RED,  GL_HALF_FLOAT);	break;
		case color_format_t::RG16F:	    fmt = opengl_pixel_format_t(GL_RG16F,   GL_RG,   GL_HALF_FLOAT);	break;
		case color_format_t::RGB16F:	fmt = opengl_pixel_format_t(GL_RGB16F,  GL_RGB,  GL_HALF_FLOAT);	break;
		case color_format_t::RGBA16F:	fmt = opengl_pixel_format_t(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);	break;
		case color_format_t::R32F:		fmt = opengl_pixel_format_t(GL_R32F,    GL_RED,  GL_FLOAT);			break;
		case color_format_t::RG32F:	    fmt = opengl_pixel_format_t(GL_RG32F,   GL_RG,   GL_FLOAT);			break;
		case color_format_t::RGB32F:	fmt = opengl_pixel_format_t(GL_RGB32F,  GL_RGB,  GL_FLOAT);			break;
		case color_format_t::RGBA32F:	fmt = opengl_pixel_format_t(GL_RGBA32F, GL_RGBA, GL_FLOAT);			break;
	}

	return fmt;
}

//
static inline uint32_t get_pixel_fmt_channels(const color_format_t& _fmt)
{
	switch (_fmt)
	{
		case color_format_t::NONE:		return 0;
		case color_format_t::R8:
		case color_format_t::R16F:
		case color_format_t::R32F:		return 1;
		case color_format_t::RG8:
		case color_format_t::RG16F:		
		case color_format_t::RG32F:	    return 2;
		case color_format_t::RGB8:
		case color_format_t::RGB16F:	
		case color_format_t::RGB32F:	return 3;
		case color_format_t::RGBA8:
		case color_format_t::RGBA16F:		
		case color_format_t::RGBA32F:	return 4;
	}
}


//
static inline size_t get_pixel_fmt_type_size(const color_format_t& _fmt)
{
	switch (_fmt)
	{
		case color_format_t::NONE:		return 0;
		case color_format_t::R8:			
		case color_format_t::RG8:		
		case color_format_t::RGB8:		
		case color_format_t::RGBA8:	    return sizeof(GLubyte);
		case color_format_t::R16F:
		case color_format_t::RG16F:
		case color_format_t::RGB16F:
		case color_format_t::RGBA16F:
		case color_format_t::R32F:
		case color_format_t::RG32F:
		case color_format_t::RGB32F:
		case color_format_t::RGBA32F:	return sizeof(GLfloat);
	}
}

// 
struct framebuffer_handle_t {
    uint32_t id = 0;
    bool is_active() const { return id != 0; }
    bool operator==(const framebuffer_handle_t &_other) { return id == _other.id; }
};





#endif // __FRAMEBUFFER_TYPES_H

#ifndef __FRAMEBUFFER_H
#define __FRAMEBUFFER_H

#include <string>
#include <glm/glm.hpp>

#include "external/glad/glad.h"

// #include "renderer/material/texture.h"
#include "renderer/buffers/framebuffer_types.h"

// 
class framebuffer_base_t
{
public:
    friend class framebuffer_handler_t;
    
public:
	framebuffer_base_t() {};
	virtual ~framebuffer_base_t();

	/* Binds the Framebuffer as the current GL_FRAMEBUFFER. */
	virtual void bind(bool _set_viewport=true);
	/* Unbinds, through binding GL_FRAMEBUFFER to 0. */
	virtual void unbind();
	/* Unbinds, through binding GL_FRAMEBUFFER to 0. */
	virtual inline void bind_default_framebuffer() { unbind(); }

	virtual void save_as_png(const std::string &_file_path="");

protected:
	/* Called on Syn::event_viewport_resize_t and also upon instantiation of this
	 * Framebuffer (called from constructor). The _depth flag controls if creation
	 * of a depth and stencil buffer should be omitted. The private variable
	 * m_hasDepthAttachment is used. The size, if not specified (ie glm::ivec2(0)),
	 * will default to Syn::Renderer::getViewport().
	 * 
	 * Update: Depth buffer flag set at (m_hasDepthBuffer) contruction and used in
	 * resize() to control the creation of a depth buffer.
	 * Update: Removed this function as initializer of the class, created separate
	 * init() private function for this.
	 */
	virtual void resize(const glm::ivec2 &_size);
	
	/* Initialization of class. See resize() for more information. */
	virtual void init(const glm::ivec2 &_size);

public:
	/* Binds specified COLOR_ATTACHMENT_N (_color_attachment_slot) to the specified
	 * slot GL_TEXTURE0+_tex_slot as a GL_TEXTURE_2D.
	 */
	virtual void bind_texture(uint32_t _tex_slot=0, GLuint _color_attachment_slot=0) const;
	/* Same as above, but with the added option of changing the interpolation
	 * parameters.
	 */
	virtual void bind_texture(uint32_t _tex_slot, GLuint _color_attachment_slot, GLint _interpolation) const;

	/* Clears buffer using Syn::Renderer::getClearColor(). */
	virtual void clear(uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) const;
	/* Clears with specified rgba _clear_color. */
	virtual void clear(const glm::vec4 &_clear_color, 
					   uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) const;

	/* Accessors */
	GLuint  get_framebuffer_id()                { 	return m_framebuffer_id; 		    }  
	GLuint* get_color_attachment_ids() 		    { 	return m_color_attachment_ids; 	    }
	GLuint  get_color_attachment_id(size_t _id) { 	return m_color_attachment_ids[_id]; }
	GLuint  get_depth_attachment_id() 			{ 	return m_depth_attachment_id; 	    }

	const glm::ivec2       &get_size() 	 { return m_size; 	}
	uint32_t 			    get_width()  { return m_size.x; }
	uint32_t 			    get_height() { return m_size.y; }
	const color_format_t& 	get_format() { return m_format; }
	const std::string      &get_name() 	 { return m_name; 	}

protected:
	GLuint 	m_framebuffer_id 			= 0;			// ID of Framebuffer
	GLuint *m_color_attachment_ids 		= nullptr;		// Multiple attachments allowed, ie. when 
														// multiple draw targets are needed.
	size_t  m_color_attachment_count	= 1;			// Number of color attachments.
	GLuint  m_depth_attachment_id 		= 0;			// ID of depth buffer.
	bool	m_has_depth_attachment		= true;			// Flag used in resize() to determine depth
														// buffer creation.
	GLuint m_color_channel = GL_COLOR_ATTACHMENT0;

	glm::ivec2 m_size = glm::ivec2(-1);				    // Size (in px), if not set through during
													    // construction defaults to Syn::Renderer
													    // viewport size.

	color_format_t m_format = color_format_t::NONE;		// Pixel format and precision (eg RGBA16F).
	opengl_pixel_format_t m_pixel_format;		        // OpenGL internal px format.

	std::string m_name = "";						    // String ID, used for debug.
	
};


//-----------------------------------------------------------------------------------
/* Main instantiation of base class. */
class framebuffer_t : public framebuffer_base_t
{
public:
    using framebuffer_base_t::resize;
    
public:
    framebuffer_t() = default;
    
	void create(const color_format_t& _format=color_format_t::RGBA16F, 
				const glm::ivec2& _size=glm::ivec2(0),
				size_t _n_drawbuffers=1,
				bool _use_depthbuffer=true,
				const std::string& _name="");

};


#endif // __FRAME_BUFFER_H
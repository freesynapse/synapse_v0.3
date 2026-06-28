
#include <algorithm>

#include "external/stb/stb_image_write.h"

#include "renderer/buffers/framebuffer.h"
#include "utils/random.h"
#include "utils/time.h"
#include "utils/log.h"

#include "c_api.h"


// 
framebuffer_base_t::~framebuffer_base_t()
{
    destroy();
}

// 
void framebuffer_base_t::destroy()
{
    glDeleteFramebuffers(1, &m_framebuffer_id);
	
	if (m_color_attachment_ids != nullptr)
		delete[] m_color_attachment_ids;
    
}

//
void framebuffer_base_t::bind(bool _set_viewport)
{
	if (_set_viewport)
		glViewport(0, 0, m_size.x, m_size.y);
		
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

}

//
void framebuffer_base_t::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//
void framebuffer_base_t::save_as_png(const std::string &_file_path/* ="" */)
{
	// size of texture in bytes, using 3 channels (rgb)
	uint32_t img_size = m_size.x * m_size.y * 3;
	unsigned char* pixels = new unsigned char[img_size];

	// format file_name
	std::string file_name;
	if (strcmp(_file_path.c_str(), "") == 0)
	{
		std::string dir_name = "./";
			
		std::string time = current_time();
		std::replace(time.begin(), time.end(), ':', '.');
		file_name = dir_name + current_date() + '_' + time + '_' + rng.rand_str(24) + ".png";

	}
	else
		file_name = _file_path;

		
	// first, bind this buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

	// glReadPixels can align the first pixel in each row at 1-, 2-, 4- and 8-byte boundaries. We
	// have allocated the exact size needed for the image so we have to use 1-byte alignment
	// (otherwise glReadPixels would write out of bounds)
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, m_size.x, m_size.y, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// unbind buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// write image data
	stbi_flip_vertically_on_write(1);
	stbi_write_png(file_name.c_str(), m_size.x, m_size.y, 3, pixels, 0);

	// cleanup
	delete[] pixels;

	SYN_INFO("screenshot saved to '%s'.\n", file_name.c_str());

}

//
void framebuffer_base_t::init(const glm::ivec2 &_size)
{
	glm::ivec2 size = (_size == glm::ivec2(0) ? api.get_viewport() : _size);
	resize(size);
	
}	

//
void framebuffer_base_t::resize(const glm::ivec2 &_size)
{
	if (_size.x == m_size.x && _size.y == m_size.y)
		return;
	else
		m_size = _size;

	if (m_framebuffer_id) {
		glDeleteFramebuffers(1, &m_framebuffer_id);
		glDeleteTextures(m_color_attachment_count, m_color_attachment_ids);
		glDeleteTextures(1, &m_depth_attachment_id);
	}

	glGenFramebuffers(1, &m_framebuffer_id);

	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

	// generate texture(s) as rendering target
	glGenTextures(m_color_attachment_count, m_color_attachment_ids);
	GLenum drawBufferIDs[m_color_attachment_count];

	for (size_t i = 0; i < m_color_attachment_count; i++) {
		glBindTexture(GL_TEXTURE_2D, m_color_attachment_ids[i]);

		// set texture with correct formats and type based on the FramebufferFormat specification.
		glTexImage2D(GL_TEXTURE_2D, 
			         0, 
			         m_pixel_format.internalFormat, 
			         m_size.x, 
			         m_size.y, 
			         0, 
			         m_pixel_format.storageFormat, 
			         m_pixel_format.storageType, 
			         NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// attach the texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_color_attachment_ids[i], 0);
		drawBufferIDs[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glDrawBuffers(m_color_attachment_count, drawBufferIDs);

	// also need depth and stencil targets, so create a render buffer
	if (m_has_depth_attachment) {
		glGenTextures(1, &m_depth_attachment_id);
		glBindTexture(GL_TEXTURE_2D, m_depth_attachment_id);
		// dimensioning of the depth buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_size.x, m_size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		// attach to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment_id, 0);
	}

	// check for completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SYN_ERROR("framebuffer incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//
void framebuffer_base_t::bind_texture(uint32_t _tex_slot, GLuint _color_attachment_slot) const
{
	glActiveTexture(GL_TEXTURE0 + _tex_slot);
	glBindTexture(GL_TEXTURE_2D, m_color_attachment_ids[_color_attachment_slot]);
	//glBindTextureUnit(_tex_slot, m_color_attachment_ids[_color_attachment_slot]);
}

//
void framebuffer_base_t::bind_texture(uint32_t _tex_slot, 
								      GLuint _color_attachment_slot,
								      GLint _interpolation) const
{
	glActiveTexture(GL_TEXTURE0 + _tex_slot);
	glBindTexture(GL_TEXTURE_2D, m_color_attachment_ids[_color_attachment_slot]);
	//glBindTextureUnit(_tex_slot, m_color_attachment_ids[_color_attachment_slot]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _interpolation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _interpolation);

}

//
void framebuffer_base_t::clear(uint32_t _buffer_mask) const
{
	glm::vec4 clearColor = api.get_clear_color();
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(_buffer_mask);

}

//
void framebuffer_base_t::clear(const glm::vec4 &_clear_color, uint32_t _buffer_mask) const
{
	// store current color
	glm::vec4 renderer_clear_color = api.get_clear_color();

	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
	glClearColor(_clear_color.r, _clear_color.g, _clear_color.b, _clear_color.a);
	glClear(_buffer_mask);

	// restore clear color
	api.set_clear_color(renderer_clear_color);

}

//
void framebuffer_t::create(const color_format_t &_format,
				           const glm::ivec2 &_size,
				           size_t _n_drawbuffers,
				           bool _use_depthbuffer,
				           const std::string &_name)
{
    m_format = _format;
    m_pixel_format = getOpenGLPixelFormat(m_format);
    
    m_color_attachment_count = _n_drawbuffers;
    m_color_attachment_ids = new GLuint[m_color_attachment_count];
    m_has_depth_attachment = _use_depthbuffer;
    
    m_name = (_name.compare("") != 0) ? _name : "unknown";
    
    // create the framebuffer
    init(_size);
        
}




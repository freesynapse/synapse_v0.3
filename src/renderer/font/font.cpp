
#include <string>
#include <string.h>
#include <stdarg.h>

#include "renderer/font/font.h"
#include "core.h"
#include "utils/log.h"
#include "event/event_handler.h"
#include "utils/math_utils.h"
#include "window.h"

#include "c_api.h"


// static event callback wrappers
static void __font_on_resize_callback(const event_t &_e) { font.on_resize(_e); }

// 
void font_t::init(const char* _filename,
                  const int& _pixel_size,
                  const glm::vec2& _vp_sz)
{
    // create shader from static lib
	SYN_INFO("creating font shader.\n");

	m_shader_handle = shader_lib.load_from_file("static_font_shader", "../assets/shaders/font.glsl");
	
	// prepare rendering objects
	memset(m_render_buffer, 0, FONT_MAX_BUFFER_LENGTH);
	m_buffer_offsets.reserve(512);
	m_render_offsets.reserve(512);
	
	// register resize events
	// events.register_callback(event_type_t::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(font_t::resize_event));
	events.register_callback(event_type_t::VIEWPORT_RESIZE, __font_on_resize_callback);
   
	// initialize the text atlas texture
	init_font_atlas(_filename, _pixel_size, _vp_sz);
	
	m_viewport_size = _vp_sz;

}

// 
void font_t::destroy()
{
    SYN_INFO("deleting buffers...\n");
	glDeleteBuffers(1, &m_font_vbo);
	glDeleteVertexArrays(1, &m_font_vao);
	glDeleteTextures(1, &m_atlas_texture_id);

}

// 
int font_t::init_font_atlas(const char* _filename, const int& _pixel_size, const glm::vec2& _vp_sz)
{
	// Use program for initiation of shader attributes
	// GLuint shader_id = shader_ptr->getShaderID();
	shader_t *shader = shader_lib.get(m_shader_handle);
	glUseProgram(shader->get_id());

	m_uniform_sampler = glGetUniformLocation(shader->get_id(), "u_texture_sampler");
	m_uniform_color = glGetUniformLocation(shader->get_id(), "u_color");
	
	// Init the FreeType lib
	if (FT_Init_FreeType(&m_ft_lib)) {
		// Error::raise_error(nullptr, __func__, "FreeType could not be initialized.");
		SYN_ERROR("FreeType could not be initialized.");
		return (RETURN_FAILURE);
	}
	
	#ifdef DEBUG_FONT
	SYN_INFO("FreeType successfully initialized.")
	#endif

	//
	SYN_INFO("loading atlas from '%s'.\n", _filename);
	
	if (FT_New_Face(m_ft_lib, _filename, 0, &m_ft_face)) {
		SYN_ERROR("could not load atlas.");
		return (RETURN_FAILURE);
	}

	// Initialize variables before atlas creation
	FT_Set_Pixel_Sizes(m_ft_face, 0, _pixel_size);
	FT_GlyphSlot g = m_ft_face->glyph;
	if (_vp_sz.x <= 1.0f || _vp_sz.y <= 1.0f) {
	    auto vp = window.m_window_dim;
	    m_sx = 2.0f / (float)vp.x;
		m_sy = 2.0f / (float)vp.y;
	} 
	else {
		m_sx = 2.0f / (float)_vp_sz.x;
		m_sy = 2.0f / (float)_vp_sz.y;
	}

	unsigned int roww = 0;
	unsigned int rowh = 0;

	m_texture_width = 0;
	m_texture_height = 0;

	memset(m_chars, 0, sizeof(character_info_s) * FONT_MAX_CHAR_SET_SIZE);

	// Find the minimum size for a texture holding the complete ASCII m_charset
	for (int i = 32; i < FONT_MAX_CHAR_SET_SIZE; i++) {
		if (FT_Load_Char(m_ft_face, i, FT_LOAD_RENDER)) {
			SYN_WARNING("could not log char %c.\n", i);
			continue;
		}

		if (roww + g->bitmap.width + 1 >= 1024) {
			m_texture_width = max(m_texture_width, roww);
			m_texture_height += rowh;
			roww = 0;
			rowh = 0;
		}

		roww += g->bitmap.width + 1;
		rowh = max(rowh, g->bitmap.rows);
	}

	m_texture_width = max(m_texture_width, roww);
	m_texture_height += rowh;

	// Create a texture to hold the character set
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_atlas_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_atlas_texture_id);
	glUniform1i(m_uniform_sampler, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture_width, m_texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	// 1 byte alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Clamping to edges and linear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Paste glyphs bitmaps into the texture
	int ox = 0;
	int oy = 0;
	rowh = 0;

	for (int i = 32; i < FONT_MAX_CHAR_SET_SIZE; i++) {
		if (FT_Load_Char(m_ft_face, i, FT_LOAD_RENDER)) {
			SYN_WARNING("could not load char bitmap '%c' into texture.\n", i);
			continue;
		}

		if (ox + g->bitmap.width + 1 >= 1024) {
			oy += rowh;
			rowh = 0;
			ox = 0;
		}

		glTexSubImage2D(GL_TEXTURE_2D,
			            0,
			            ox,
			            oy,
			            g->bitmap.width,
			            g->bitmap.rows,
			            GL_RED,
			            GL_UNSIGNED_BYTE,
			            g->bitmap.buffer);

		m_chars[i].ax = (float)(g->advance.x >> 6);
		m_chars[i].ay = (float)(g->advance.y >> 6);
		m_chars[i].bw = (float)g->bitmap.width;
		m_chars[i].bh = (float)g->bitmap.rows;
		m_chars[i].bl = (float)g->bitmap_left;
		m_chars[i].bt = (float)g->bitmap_top;
		m_chars[i].tx = ox / (float)m_texture_width;
		m_chars[i].ty = oy / (float)m_texture_height;

		rowh = max(rowh, g->bitmap.rows);
		ox += g->bitmap.width + 1;
	}

	// Generate VAO
	glGenVertexArrays(1, &m_font_vao);
	glBindVertexArray(m_font_vao);

	// Generate the VBO for fonts
	glGenBuffers(1, &m_font_vbo);

	// unbind vertex array
	glBindVertexArray(0);

	SYN_INFO("generated %dx%d text atlas.\n", m_texture_width, m_texture_height);

	return RETURN_SUCCESS;

}

// 
void font_t::begin_render_block()
{
	// clear everything
	memset(m_render_buffer, 0, FONT_MAX_BUFFER_LENGTH);
	m_buffer_offsets.clear();
	m_buffer_offsets.push_back(0);
	m_buffer_len = 0;
	m_render_offsets.clear();
}

// 
void font_t::end_render_block()
{
    if (m_render_offsets.size() == 0) {
        return;
    }

    renderer.set_depth_testing(false);
	
	shader_t *shader = shader_lib.get(m_shader_handle);
	shader->enable();
	
	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_atlas_texture_id);
	glUniform1i(m_uniform_sampler, 0);

	// Select the font VBO
	glBindVertexArray(m_font_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_font_vbo);
	glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_POSITION);
	glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_POSITION, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

	uint32_t c = 0;
	
	
	for (size_t i = 0; i < m_render_offsets.size(); i++) {
		glm::vec2& pos = m_render_offsets[i];
		float x = -1 + pos.x * m_sx;
		float y =  1 - pos.y * m_sy;

		size_t start_j = m_buffer_offsets[i];
        size_t end_j = m_buffer_offsets[i+1];
      		
        if (i + 1 >= m_buffer_offsets.size()) {
		    break;
		}

		if (start_j >= FONT_MAX_BUFFER_LENGTH) continue;
        if (end_j > FONT_MAX_BUFFER_LENGTH) end_j = FONT_MAX_BUFFER_LENGTH;
		
		// Loop through all characters
		for (size_t j = m_buffer_offsets[i]; j < m_buffer_offsets[i+1]; j++) {
            if (m_render_buffer[j] == '\0') {
                break;
            }
            
            if (c + 6 >= FONT_MAX_BUFFER_LENGTH * 6) {
                break; 
			}
		
		    uint8_t ascii_val = (uint8_t)m_render_buffer[j];
			if (ascii_val >= FONT_MAX_CHAR_SET_SIZE) {
                continue;
			}
			
			// calculate vertex and texture coordinates
			float x2 =  x + m_chars[ascii_val].bl * m_sx;
			float y2 = y + m_chars[ascii_val].bt * m_sy;
			float w = m_chars[ascii_val].bw * m_sx;
			float h = m_chars[ascii_val].bh * m_sy;

			// advance cursor
			x += m_chars[ascii_val].ax * m_sx;
			y += m_chars[ascii_val].ay * m_sy;

			// skip empty m_chars
			if (!w || !h)
				continue;

            m_texture_coords[c + 0] = font_point_t(x2 + w,  y2,     m_chars[ascii_val].tx + m_chars[ascii_val].bw / (float)m_texture_width, m_chars[ascii_val].ty);
            m_texture_coords[c + 1] = font_point_t(x2,      y2,     m_chars[ascii_val].tx,                                                  m_chars[ascii_val].ty);
            m_texture_coords[c + 2] = font_point_t(x2,      y2 - h, m_chars[ascii_val].tx,                                                  m_chars[ascii_val].ty + m_chars[ascii_val].bh / (float)m_texture_height);
            
            m_texture_coords[c + 3] = font_point_t(x2 + w,  y2,     m_chars[ascii_val].tx + m_chars[ascii_val].bw / (float)m_texture_width, m_chars[ascii_val].ty);
            m_texture_coords[c + 4] = font_point_t(x2,      y2 - h, m_chars[ascii_val].tx,                                                  m_chars[ascii_val].ty + m_chars[ascii_val].bh / (float)m_texture_height);
            m_texture_coords[c + 5] = font_point_t(x2 + w,  y2 - h, m_chars[ascii_val].tx + m_chars[ascii_val].bw / (float)m_texture_width, m_chars[ascii_val].ty + m_chars[ascii_val].bh / (float)m_texture_height);
            
            c += 6;			
		}
	}

	if (c > 0) {
    	glBufferData(GL_ARRAY_BUFFER, sizeof(font_point_t) * c, m_texture_coords, GL_DYNAMIC_DRAW);
    	glDrawArrays(GL_TRIANGLES, 0, c);
	}

	glDisableVertexAttribArray(VERTEX_ATTRIB_LOCATION_POSITION);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//
	shader->disable();
	renderer.set_depth_testing(true);

}

// 
void font_t::render_text(const float& _x, const float& _y, const char* _str, ...)
{
	va_list arglist;

	if (!_str)
		return;

	// set the new buffer and store the incremented offset
	memset(m_tmp_buffer, 0, 1024);
	va_start(arglist, _str);
	int str_len = vsprintf(m_tmp_buffer, _str, arglist);
	va_end(arglist);

	// cpy to static buffer
	memcpy(m_render_buffer + m_buffer_len, m_tmp_buffer, str_len);
	m_buffer_len += str_len;

	// store render coordinates and new buffer offset
	m_render_offsets.push_back(glm::vec2(_x, _y));
	m_buffer_offsets.push_back(m_buffer_len);

}

// 
float font_t::get_string_width(const char* _str, ...)
{
	memset(m_tmp_buffer, 0, 256);

	va_list arglist;
	va_start(arglist, _str);
	int offset = vsprintf(m_tmp_buffer, _str, arglist);
	va_end(arglist);

	return offset * m_chars[m_tmp_buffer[0]].ax;
}

// 
void font_t::set_color(const glm::vec4& _color)
{
    m_text_color = _color;

    shader_t *shader = shader_lib.get(m_shader_handle);
	shader->enable();
	glUniform4fv(m_uniform_color, 1, (GLfloat*)(&m_text_color));
	shader->disable();
}

// 
void font_t::on_resize(const event_t &_e)
{
	if (!m_update_on_resize)
		return;

	glm::ivec2 new_viewport = _e.as.viewport_resize.viewport;
	
	m_sx = 2.0f / (float)new_viewport.x;
	m_sy = 2.0f / (float)new_viewport.y;
	
}

// 
void font_t::resize(const glm::vec2& _vp_sz_px)
{
	m_sx = 2.0f / _vp_sz_px.x;
	m_sy = 2.0f / _vp_sz_px.y;
}

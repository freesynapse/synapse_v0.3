
#include <string>
#include <string.h>
#include <stdarg.h>

#include "renderer/font/font.h"

#include "utils/log.h"
#include "utils/math_utils.h"
#include "glfw_window.h"

#include "c_api.h"


//
void font_t::init(const char *_filename, const int &_pixel_size, const glm::vec2 &_vp_sz)
{
	m_shader_handle = shader_lib.load_from_file("font_shader", "../assets/shaders/font.glsl");

	// initialize the text atlas texture
	init_font_atlas(_filename, _pixel_size, _vp_sz);

	m_viewport_size = _vp_sz;

	// 6 vertices per character
	m_vertices.reserve(SYN_FONT_MAX_BUFFER_LENGTH * 6);

}

//
void font_t::destroy()
{
    m_vao.destroy();
    glDeleteTextures(1, &m_atlas_texture_id);

}

//
int font_t::init_font_atlas(const char* _filename, const int &_pixel_size, const glm::vec2 &_vp_sz)
{
	// Init the FreeType lib
	if (FT_Init_FreeType(&m_ft_lib)) {
		SYN_ERROR("FreeType could not be initialized.\n");
		return (-1);
	}

	SYN_INFO("loading font atlas from '%s'.\n", _filename);

	if (FT_New_Face(m_ft_lib, _filename, 0, &m_ft_face)) {
		SYN_ERROR("could not font load atlas from '%s'.\n", _filename);
		return (-1);
	}

	// Initialize variables before atlas creation
	FT_Set_Pixel_Sizes(m_ft_face, 0, _pixel_size);
	FT_GlyphSlot g = m_ft_face->glyph;
	FT_Load_Char(m_ft_face, 'H', FT_LOAD_RENDER);
	m_font_ascender = (float)m_ft_face->glyph->bitmap.rows;

	unsigned int roww = 0;
	unsigned int rowh = 0;

	m_texture_width = 0;
	m_texture_height = 0;

	memset(m_chars, 0, sizeof(character_info_s) * SYN_FONT_MAX_CHAR_SET_SIZE);

	// Find the minimum size for a texture holding the complete ASCII m_charset
	for (int i = 32; i < SYN_FONT_MAX_CHAR_SET_SIZE; i++) {
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
	glGenTextures(1, &m_atlas_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_atlas_texture_id);

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

	for (int i = 32; i < SYN_FONT_MAX_CHAR_SET_SIZE; i++) {
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

	m_vao.set_buffer_layout({
	    { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
		{ VERTEX_ATTRIB_LOCATION_UV, shader_data_type_t::FLOAT2 },
		{ VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
		{ VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
	});
	m_vao.create_empty_vertices(6 * sizeof(font_vertex_t) * SYN_FONT_MAX_BUFFER_LENGTH, GL_DYNAMIC_DRAW);

	SYN_INFO("generated %dx%d text atlas.\n", m_texture_width, m_texture_height);

	return 0;

}

//
void font_t::render_text(const float &_x, const float &_y, const char* _str, ...)
{
    if (!_str) return;

	//
	va_list arglist;
	memset(m_tmp_buffer, 0, SYN_FONT_MAX_STRING_LENGTH);
	va_start(arglist, _str);
	size_t str_len = vsnprintf(m_tmp_buffer, SYN_FONT_MAX_STRING_LENGTH, _str, arglist);
	va_end(arglist);

// 	render_text(_x, _y, m_tmp_buffer);
// }

// // 
// void font_t::render_text(const float &_x, const float &_y, const char* _str)
// {
	// add vertices to the buffer
	float x = std::round(_x);
	float y = std::round(_y);
	
	for (size_t i = 0; i < str_len; i++) {
    	if (m_tmp_buffer[i] == '\0' || m_tmp_buffer[i] == '\n') break;

        uint8_t ascii_val = (uint8_t)m_tmp_buffer[i];
        if (ascii_val < 32 || ascii_val >= SYN_FONT_MAX_CHAR_SET_SIZE) continue;

        // calculate vertex and texture coordinates
        character_info_s c = m_chars[ascii_val];
        float x2 = std::round(x + c.bl);
        float y2 = std::round(y - c.bt);
        float w = c.bw;
        float h = c.bh;

        // advance cursor
        x += c.ax;
        y += c.ay;

        // skip empty m_chars
        if (!w || !h) continue;

        float u0 = c.tx;
        float v0 = c.ty;
        float u1 = c.tx + c.bw / (float)m_texture_width;
        float v1 = c.ty + c.bh / (float)m_texture_height;

        m_vertices.push_back(font_vertex_t({ x2,     y2 + h }, { u0, v1 }, m_text_color, m_current_depth));  // bottom-left
        m_vertices.push_back(font_vertex_t({ x2 + w, y2 + h }, { u1, v1 }, m_text_color, m_current_depth));  // bottom-right
        m_vertices.push_back(font_vertex_t({ x2 + w, y2     }, { u1, v0 }, m_text_color, m_current_depth));  // top-right
        m_vertices.push_back(font_vertex_t({ x2,     y2 + h }, { u0, v1 }, m_text_color, m_current_depth));
        m_vertices.push_back(font_vertex_t({ x2 + w, y2     }, { u1, v0 }, m_text_color, m_current_depth));
        m_vertices.push_back(font_vertex_t({ x2,     y2     }, { u0, v0 }, m_text_color, m_current_depth));  // top-left
        
	}
}
  
//
void font_t::render_text_clipped(const float &_x, const float &_y, float _max_width, const char *_str, ...)
{
    if (!_str) return;

    va_list arglist;
    memset(m_tmp_buffer, 0, SYN_FONT_MAX_STRING_LENGTH);
    va_start(arglist, _str);
    size_t str_len = vsnprintf(m_tmp_buffer, SYN_FONT_MAX_STRING_LENGTH, _str, arglist);
    va_end(arglist);

    float x = std::round(_x);
    float y = std::round(_y);
    float clip_x = _x + _max_width;

    for (size_t i = 0; i < str_len; i++) {
        if (m_tmp_buffer[i] == '\0' || m_tmp_buffer[i] == '\n') break;

        uint8_t ascii_val = (uint8_t)m_tmp_buffer[i];
        if (ascii_val < 32 || ascii_val >= SYN_FONT_MAX_CHAR_SET_SIZE) continue;

        character_info_s c = m_chars[ascii_val];
        float x2 = std::round(x + c.bl);
        float y2 = std::round(y - c.bt);
        float w = c.bw;
        float h = c.bh;

        if (x2 >= clip_x) return;

        x += c.ax;
        y += c.ay;

        if (!w || !h) continue;

        float u0 = c.tx;
        float v0 = c.ty;
        float u1 = c.tx + c.bw / (float)m_texture_width;
        float v1 = c.ty + c.bh / (float)m_texture_height;

        m_vertices.push_back(font_vertex_t({ x2,     y2 + h }, { u0, v1 }, m_text_color, m_current_depth));  // bottom-left
        m_vertices.push_back(font_vertex_t({ x2 + w, y2 + h }, { u1, v1 }, m_text_color, m_current_depth));  // bottom-right
        m_vertices.push_back(font_vertex_t({ x2 + w, y2     }, { u1, v0 }, m_text_color, m_current_depth));  // top-right
        m_vertices.push_back(font_vertex_t({ x2,     y2 + h }, { u0, v1 }, m_text_color, m_current_depth));
        m_vertices.push_back(font_vertex_t({ x2 + w, y2     }, { u1, v0 }, m_text_color, m_current_depth));
        m_vertices.push_back(font_vertex_t({ x2,     y2     }, { u0, v0 }, m_text_color, m_current_depth));  // top-left
    }
}

//
void font_t::end_render_block(bool _use_depth_test)
{    
    if (m_vertices.empty()) return;

    if (!_use_depth_test) {
        api.disable_depth_test();
    }

    glm::mat4 proj = glm::ortho(0.0f, root_window.get_fwidth(), 
                                root_window.get_fheight(), 0.0f, 
                                window_manager.get_zfar(), window_manager.get_znear());
    
    
    shader_t *shader = shader_lib.get_shader(m_shader_handle);
	shader->enable();
	shader->set_matrix_4fv("u_projection", proj);
    
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_atlas_texture_id);

	m_vao.bind();
	m_vao.update_vertices(&m_vertices[0], sizeof(font_vertex_t) * m_vertices.size(), 0);	
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
    m_vao.unbind();

    renderer.m_perf_stats.draw_calls_per_frame++;
    
    shader->disable();

    if (!_use_depth_test) {
        api.enable_depth_test();
    }

	m_vertices.clear();
}

//
float font_t::get_string_width(const char* _str, ...)
{
	memset(m_tmp_buffer, 0, 256);

	va_list arglist;
	va_start(arglist, _str);
	int offset = vsprintf(m_tmp_buffer, _str, arglist);
	va_end(arglist);

	// monospaced font required here...
	return offset * m_chars[(uint32_t)m_tmp_buffer[0]].ax;
}


#ifndef __FONT_H
#define __FONT_H

#include <vector>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include "renderer/buffers/vertex_array.h"
#include "event/event.h"

// 
struct font_vertex_t {
	glm::vec2 position;
	glm::vec2 uv;
	glm::vec4 color;
	float depth;

	font_vertex_t(const glm::vec2 _pos, const glm::vec2 _uv, const glm::vec4 _color, float _depth) :
        position(_pos), uv(_uv), color(_color), depth(_depth) {}
    font_vertex_t() {}
};

// 
struct character_info_s {
	float ax, ay;
	float bw, bh;
	float bl, bt;
	float tx, ty;
};

// 
#define SYN_FONT_MAX_BUFFER_LENGTH  8192
#define SYN_FONT_MAX_STRING_LENGTH   512
#define SYN_FONT_MAX_CHAR_SET_SIZE   128

// 
class font_t
{
public:
	font_t() = default;
	~font_t() = default;
	
	void init(const char *_filename, const int &_pixel_size=12, const glm::vec2 &_vp_sz=glm::vec2(0.0f));
	void destroy();
	
	void begin_render_block();
	void end_render_block(bool _use_depth_test);
	void end_render_block_with_proj(const glm::mat4 &_proj);
	void render_text(const float &_x, const float &_y, const char *_str, ...);
	void render_text_clipped(const float &_x, const float &_y, float _max_width, const char *_str, ...);
	void render_text_centered(const float &_x, const float &_y, const char *_str, ...);
	void render_text_right(const float &_x, const float &_y, const char *_str, ...);
	
	// in pixels
	float get_string_width(const char *_str, ...);
	// float get_string_width(const char *_str);
	void set_color(const glm::vec4 &_color) { m_text_color = _color; }
	void set_depth(float _depth) { m_current_depth = _depth; }
	float get_current_depth() { return m_current_depth; }
	const glm::vec4 &get_color() { return m_text_color; }
	float get_font_height() { return (float)(m_texture_height); }
	float get_font_glyph_height() { return (float)(m_font_ascender); }
	const std::vector<font_vertex_t> &get_vertices() { return m_vertices; }
	
private:
	int init_font_atlas(const char *_filename, const int &_pixel_size, const glm::vec2 &_vp_sz);
	void render_text_raw(const float &_x, const float &_y, const char *_str);
	
private:
	// FreeType
	FT_Library m_ft_lib;
	FT_Face m_ft_face;
	FT_GlyphSlot m_glyph;

	// texture
	GLuint m_atlas_texture_id = 0;
	unsigned int m_texture_width = 0;
	unsigned int m_texture_height = 0;
	
	// atlas
	char m_tmp_buffer[SYN_FONT_MAX_STRING_LENGTH];
	std::vector<font_vertex_t> m_vertices;
	character_info_s m_chars[SYN_FONT_MAX_CHAR_SET_SIZE];
	float m_font_ascender;  // pixels from base to top of the tallest glyph

	// GLSL shaders
	shader_handle_t m_shader_handle = { 0 };
	vertex_array_t m_vao;
	glm::vec4 m_text_color = glm::vec4(1.0f);
	float m_current_depth = 0.0f;
	
	// rendering parameters
	glm::vec2 m_viewport_size = { 0.0f, 0.0f };
	bool m_update_on_resize = true;

};


#endif // __FONT_H

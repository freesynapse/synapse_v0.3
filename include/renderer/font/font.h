#ifndef __FONT_H
#define __FONT_H

// #pragma warning(disable : 4005)

#include <vector>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include "event/event.h"

// 
struct font_point_t
{
	GLfloat x, y, s, t;
	font_point_t(float _x, float _y, float _s, float _t) : x(_x), y(_y), s(_s), t(_t) {}
	font_point_t() : x(0.0f), y(0.0f), s(0.0f), t(1.0f) {}
};

// 
struct character_info_s
{
	float ax, ay;
	float bw, bh;
	float bl, bt;
	float tx, ty;
};

// 
#define FONT_MAX_BUFFER_LENGTH  8192
#define FONT_MAX_CHAR_SET_SIZE   128

// 
class font_t
{
public:
	font_t() {};
	~font_t() = default;
	
	void init(const char *_filename, const int& _pixel_size=12, const glm::vec2& _vp_sz=glm::vec2(0.0f));
	void destroy();
	
	void begin_render_block();
	void end_render_block();
	void render_text(const float& _x, const float& _y, const char* _str, ...);
	// in pixels
	float get_string_width(const char* _str, ...);
	void set_color(const glm::vec4& _color);
	const glm::vec4 &get_color() { return m_text_color; }
	
	// Accessors
	inline float get_font_height() { return (float)(m_texture_height); }
	
	void on_resize(const event_t &_e);
	void resize(const glm::vec2& _vp_sz_px);

private:
	int init_font_atlas(const char *_filename, const int& _pixel_size, const glm::vec2& _vp_sz);

private:
	// FreeType
	FT_Library m_ft_lib;
	FT_Face m_ft_face;
	FT_GlyphSlot m_glyph;

	// texture
	GLuint m_font_texture_id = 0;
	GLuint m_atlas_texture_id = 0;
	unsigned int m_texture_width = 0;
	unsigned int m_texture_height = 0;
	//GLint *m_swizzleMask;

	// atlas
	char m_render_buffer[FONT_MAX_BUFFER_LENGTH];
	char m_tmp_buffer[1024];
	std::vector<uint32_t> m_buffer_offsets;
	uint32_t m_buffer_len;
	std::vector<glm::vec2> m_render_offsets;
	character_info_s m_chars[FONT_MAX_CHAR_SET_SIZE];
	font_point_t m_texture_coords[FONT_MAX_BUFFER_LENGTH * 6];

	// vbo
	GLuint m_font_vao = 0;
	GLuint m_font_vbo = 0;

	// GLSL shaders
	shader_handle_t m_shader_handle = { 0 };
	GLuint m_uniform_sampler = 0;
	GLuint m_uniform_color = 0;
	glm::vec4 m_text_color = glm::vec4(1.0f);

	// .text rendering attributes
	glm::vec2 m_viewport_size = { 0.0f, 0.0f };
	bool m_update_on_resize = true;
	float m_sx = 0.0f;
	float m_sy = 0.0f;
};


#endif // __FONT_H

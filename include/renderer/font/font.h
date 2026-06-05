#ifndef __FONT_H
#define __FONT_H

// #pragma warning(disable : 4005)

#include <vector>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include "renderer/buffers/vertex_array.h"
#include "event/event.h"

// 
struct font_vertex_t
{
	glm::vec2 position;
	glm::vec2 uv;
	glm::vec4 color;

	font_vertex_t(const glm::vec2 _pos, const glm::vec2 _uv, const glm::vec4 _color) :
        position(_pos), uv(_uv), color(_color) {}
	// font_vertex_t(float _x, float _y, float _u, float _v) : 
	    // position({ _x, _y }), uv({ _u, _v }), color(glm::vec4(1.0f)) {}
	// font_vertex_t() : position(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), color(glm::vec4(1.0f)) {}

	// glm::vec4 color;

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
#define SYN_FONT_MAX_BUFFER_LENGTH  8192
#define SYN_FONT_MAX_STRING_LENGTH   512
#define SYN_FONT_MAX_CHAR_SET_SIZE   128

// 
class font_t
{
public:
	font_t() = default;
	~font_t() = default;
	
	void init(const char *_filename, const int& _pixel_size=12, const glm::vec2& _vp_sz=glm::vec2(0.0f));
	void destroy();
	
	void begin_render_block();
	void end_render_block();
	void render_text(const float& _x, const float& _y, const char* _str, ...);
	// in pixels
	float get_string_width(const char* _str, ...);
	void set_color(const glm::vec4& _color) { m_text_color = _color; }
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
	GLuint m_atlas_texture_id = 0;
	unsigned int m_texture_width = 0;
	unsigned int m_texture_height = 0;

	// atlas
	char m_tmp_buffer[SYN_FONT_MAX_STRING_LENGTH];
	std::vector<font_vertex_t> m_vertices;
	character_info_s m_chars[SYN_FONT_MAX_CHAR_SET_SIZE];

	// GLSL shaders
	shader_handle_t m_shader_handle = { 0 };
	vertex_array_t m_vao;
	glm::vec4 m_text_color = glm::vec4(1.0f);

	// rendering parameters
	glm::vec2 m_viewport_size = { 0.0f, 0.0f };
	bool m_update_on_resize = true;
	float m_sx = 0.0f;
	float m_sy = 0.0f;
};


#endif // __FONT_H

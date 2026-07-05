#ifndef __FONT_TYPES_H
#define __FONT_TYPES_H

#include <glm/glm.hpp>


// 
#define SYN_FONT_MAX_BUFFER_LENGTH  8192
#define SYN_FONT_MAX_STRING_LENGTH   512
#define SYN_FONT_MAX_CHAR_SET_SIZE   128

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
struct codepoint_range_t {
    uint32_t start;
    uint32_t end;
};


#endif // __FONT_TYPES_H

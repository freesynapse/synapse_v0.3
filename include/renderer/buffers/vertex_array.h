#ifndef __VERTEX_ARRAY_H
#define __VERTEX_ARRAY_H

#include <string.h>
#include <vector>
#include <stdio.h>

#include <glm/glm.hpp>

#ifndef GLAD_INCLUDED
#include "external/glad/glad.h"
#endif


// 
#define SYN_MAX_BUFFER_ELEMENTS             6

// static openGL vertex attribute locations
#define VERTEX_ATTRIB_LOCATION_POSITION		0
#define VERTEX_ATTRIB_LOCATION_NORMAL		1
#define VERTEX_ATTRIB_LOCATION_TANGENT		2
#define VERTEX_ATTRIB_LOCATION_BITANGENT	3
#define VERTEX_ATTRIB_LOCATION_UV			4
#define VERTEX_ATTRIB_LOCATION_COLOR		5
#define VERTEX_ATTRIB_LOCATION_DEPTH        6

// 
enum class shader_data_type_t { 
	NONE = 0,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	MAT3,
	MAT4,
	INT,
	INT2,
	INT3,
	INT4,
};

// 
uint32_t shader_data_type_size(shader_data_type_t _type);
uint32_t get_component_count(shader_data_type_t _type);
GLenum shader_data_type_to_openGL_enum(shader_data_type_t _type);

// 
struct buffer_element_t {
	uint32_t shader_location = 0;
	shader_data_type_t type = shader_data_type_t::NONE;
	uint32_t size = 0;
	uint32_t offset = 0;

	buffer_element_t() {}
	buffer_element_t(uint32_t _shader_location, shader_data_type_t _type) :
        shader_location(_shader_location), type(_type), size(shader_data_type_size(_type)), offset(0)
	{}
};

// 
class buffer_layout_t 
{
public:
    buffer_layout_t() = default;
    buffer_layout_t(const std::initializer_list<buffer_element_t> &_elements)
	{
	    memset(m_elements, 0, sizeof(buffer_element_t) * SYN_MAX_BUFFER_ELEMENTS);
		m_element_count = 0;
		uint32_t offset = 0;
		m_stride = 0;

		//
	    for (auto &e : _elements) {
			buffer_element_t element;
			element.shader_location = e.shader_location;
			element.type = e.type;
			element.offset = offset;
			offset += e.size;
			m_stride += e.size;
			// 
			m_elements[m_element_count] = element;
			m_element_count++;
		}
	}

	buffer_element_t m_elements[SYN_MAX_BUFFER_ELEMENTS];
    uint32_t m_element_count = 0;
    uint32_t m_stride = 0;
};

// 
class vertex_array_t
{
public:
    vertex_array_t() = default;
    ~vertex_array_t() = default;

    // for optional vertex data struct
    void set_buffer_layout(const buffer_layout_t &_buffer_layout) { m_buffer_layout = _buffer_layout; }
    
    void create(const void *_vertex_data, uint32_t _vertex_count, const uint32_t *_indices=NULL, size_t _index_count=0);
    void create_empty_vertices(size_t _vertices_max_size_bytes, GLuint _mode=GL_DYNAMIC_DRAW);
    void create_empty_indices(size_t _indices_max_size_bytes, GLuint _mode=GL_DYNAMIC_DRAW);
    void destroy();
    void update_vertices(const void *_vertex_data, uint32_t _size_in_bytes, uint32_t _offset=0);
    void update_indices(const void *_index_data, uint32_t _size_in_bytes, uint32_t _offset=0);
    void bind() const;
    void unbind() const;

    __always_inline GLuint get_array_id() const { return m_array_id; }
    __always_inline uint32_t get_index_count() const { return m_index_count; }
    __always_inline uint32_t get_vertex_count() const { return m_vertex_count; }
    __always_inline GLuint get_vbo() const { return m_vbo; }
    
public:
    GLuint m_array_id = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_vertex_count = 0;
    size_t m_index_count = 0;
    buffer_layout_t m_buffer_layout;

    size_t m_vertex_data_max_size = 0;
    
};

// TODO : add buffer layouts!!



#endif // __VERTEX_ARRAY_H

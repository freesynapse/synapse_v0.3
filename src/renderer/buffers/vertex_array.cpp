
#include "renderer/buffers/vertex_array.h"
#include "renderer/mesh/mesh_types.h"
#include "utils/log.h"

// 
uint32_t shader_data_type_size(shader_data_type_t _type)
{
	switch (_type)
	{
		case shader_data_type_t::NONE:		return 0;
		case shader_data_type_t::FLOAT:		return 4;
		case shader_data_type_t::FLOAT2:	return 4 * 2;
		case shader_data_type_t::FLOAT3:	return 4 * 3;
		case shader_data_type_t::FLOAT4:	return 4 * 4;
		case shader_data_type_t::MAT3:		return 4 * 3 * 3;
		case shader_data_type_t::MAT4:		return 4 * 4 * 4;
		case shader_data_type_t::INT:		return 4;
		case shader_data_type_t::INT2:		return 4 * 2;
		case shader_data_type_t::INT3:		return 4 * 3;
		case shader_data_type_t::INT4:		return 4 * 4;
	}
	return 0;
}

// 
uint32_t get_component_count(shader_data_type_t _type)
{
	switch (_type)
	{
		case shader_data_type_t::NONE:		return 0;
		case shader_data_type_t::FLOAT:		return 1;
		case shader_data_type_t::FLOAT2:	return 2;
		case shader_data_type_t::FLOAT3:	return 3;
		case shader_data_type_t::FLOAT4:	return 4;
		case shader_data_type_t::MAT3:		return 3 * 3;
		case shader_data_type_t::MAT4:		return 4 * 4;
		case shader_data_type_t::INT:		return 1;
		case shader_data_type_t::INT2:		return 2;
		case shader_data_type_t::INT3:		return 3;
		case shader_data_type_t::INT4:		return 4;
	}
	return 0;
}

// 
GLenum shader_data_type_to_openGL_enum(shader_data_type_t _type)
{
	switch (_type)
	{
		case shader_data_type_t::FLOAT:
		case shader_data_type_t::FLOAT2:
		case shader_data_type_t::FLOAT3:
		case shader_data_type_t::FLOAT4:
		case shader_data_type_t::MAT3:		
		case shader_data_type_t::MAT4:		return GL_FLOAT;
		case shader_data_type_t::INT:
		case shader_data_type_t::INT2:
		case shader_data_type_t::INT3:
		case shader_data_type_t::INT4:		return GL_INT;
		case shader_data_type_t::NONE:		return GL_NONE;
	}
}

// 
void vertex_array_t::create(const void *_vertex_data, uint32_t _vertex_count, const uint32_t *_indices, size_t _index_count)
{
    m_vertex_count = _vertex_count;
    m_index_count = _index_count;

    glGenVertexArrays(1, &m_array_id);
    glGenBuffers(1, &m_vbo);
    if (_indices != NULL) {
        glGenBuffers(1, &m_ebo);
    }

    glBindVertexArray(m_array_id);

    // upload buffer data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // 
    if (m_buffer_layout.m_element_count == 0) {
        glBufferData(GL_ARRAY_BUFFER, _vertex_count * sizeof(vertex_data_t), _vertex_data, GL_STATIC_DRAW);
    } 
    //  buffer_element size (corresponding to m_stride)
    else {
        glBufferData(GL_ARRAY_BUFFER, _vertex_count * m_buffer_layout.m_stride, _vertex_data, GL_STATIC_DRAW);
    }

    // upload indices (if any)
    if (_indices != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _index_count * sizeof(uint32_t), _indices, GL_STATIC_DRAW);
    }

    // define vertex attributes layout
    if (m_buffer_layout.m_element_count == 0) {
        size_t stride = sizeof(vertex_data_t);
        glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_POSITION);
        glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(vertex_data_t, position));
    
        glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_NORMAL);
        glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(vertex_data_t, normal));

        glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_TANGENT);
        glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(vertex_data_t, tangent));
        
        glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_BITANGENT);
        glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_BITANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(vertex_data_t, bitangent));
        
        glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_UV);
        glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_UV, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(vertex_data_t, uv));
    }
    // a buffer layout has been set, define vertex attributes accordingly.
    else {
        for (uint32_t i = 0; i < m_buffer_layout.m_element_count; i++) 
        {
            buffer_element_t &e = m_buffer_layout.m_elements[i];
            glEnableVertexAttribArray(e.shader_location);
            glVertexAttribPointer(e.shader_location,
                                  get_component_count(e.type),
                                  shader_data_type_to_openGL_enum(e.type),
                                  GL_FALSE,
                                  m_buffer_layout.m_stride,
                                  (const void *)(size_t)e.offset);
        }
        
    }
    
    // break bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (_indices != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }    
}

// 
void vertex_array_t::create_empty_vertices(size_t _vertices_max_size_bytes, GLuint _mode/*=GL_DYNAMIC_DRAW*/)
{
    glGenVertexArrays(1, &m_array_id);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, _vertices_max_size_bytes, NULL, _mode);

    if (m_buffer_layout.m_element_count == 0) {
        SYN_FATAL_ERROR("no buffer layout set for emtpy vertex_array_t.\n");
        return;
    }

    for (uint32_t i = 0; i < m_buffer_layout.m_element_count; i++) 
    {
        buffer_element_t &e = m_buffer_layout.m_elements[i];
        glEnableVertexAttribArray(e.shader_location);
        glVertexAttribPointer(e.shader_location,
                              get_component_count(e.type),
                              shader_data_type_to_openGL_enum(e.type),
                              GL_FALSE,
                              m_buffer_layout.m_stride,
                              (const void *)(size_t)e.offset);
    }

    glBindVertexArray(0);
    
}

// 
void vertex_array_t::create_empty_indices(size_t _indices_max_size_bytes, GLuint _mode/*=GL_DYNAMIC_DRAW*/)
{
    if (!m_array_id) {
        glGenVertexArrays(1, &m_array_id);
    }

    glBindVertexArray(m_array_id);
    
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices_max_size_bytes, NULL, _mode);
    
    glBindVertexArray(0);
    
}

// 
void vertex_array_t::destroy()
{
    if (m_array_id) glDeleteVertexArrays(1, &m_array_id);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    m_array_id = 0;
    m_vbo = 0;
    m_ebo = 0;
    m_vertex_count = 0;
    m_index_count = 0;
}

// 
void vertex_array_t::update_vertices(const void *_vertex_data, 
                                     uint32_t _size_in_bytes, 
                                     uint32_t _offset)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, _offset, _size_in_bytes, _vertex_data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
}

// 
void vertex_array_t::update_indices(const void *_index_data, 
                                    uint32_t _size_in_bytes, 
                                    uint32_t _offset)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _offset, _size_in_bytes, _index_data);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

// 
void vertex_array_t::bind() const
{
    glBindVertexArray(m_array_id);
}

// 
void vertex_array_t::unbind() const
{
    glBindVertexArray(0);
}






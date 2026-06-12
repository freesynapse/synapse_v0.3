#ifndef __UI_RENDER_BATCH_H
#define __UI_RENDER_BATCH_H

#include "renderer/buffers/vertex_array.h"
#include "renderer/shader/shader_types.h"
#include "renderer/UI/ui_types.h"

// 
#define SYN_UI_BATCH_MAX_QUADS  1024
#define SYN_UI_BATCH_MAX_LINES  1024

// 
class ui_render_batch_t
{
public:
    ui_render_batch_t() = default;
    ~ui_render_batch_t() = default;
    
    void init();
    void shutdown();

    void begin_batch();
    void add_quad(const glm::vec2 &_position, const glm::vec2 &_size, const glm::vec4 &_color, float _depth);
    void add_line_strip(ui_render_vertex_t *_vertices, size_t _vertex_count);
    void end_batch();

    bool is_initalized() { return m_is_initialized; }
    
private:
    ui_render_vertex_t m_quad_vertices[SYN_UI_BATCH_MAX_QUADS * 4];
    size_t m_quad_vertex_count = 0;

    uint32_t m_quad_indices[SYN_UI_BATCH_MAX_QUADS * 6];
    size_t m_quad_index_count = 0;

    // assumes a line strip dont (on average) contain more than 8 vertices
    ui_render_vertex_t m_line_vertices[SYN_UI_BATCH_MAX_LINES * 8];
    size_t m_line_vertex_count;

    // same as above, adding in the glPrimitiveRestartIndex(0xFFFFFFFF) for separating 
    // adjecent GL_LINE_STRIP
    uint32_t m_line_indices[SYN_UI_BATCH_MAX_LINES * 9];
    size_t m_line_index_count;
    
    vertex_array_t m_vao_quads;
    vertex_array_t m_vao_lines;
    
    shader_handle_t m_shader_handle;

    bool m_is_initialized = false;
    
};



#endif // __UI_QUAD_BATCH_H

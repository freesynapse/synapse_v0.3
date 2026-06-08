
#include "renderer/UI/window/ui_render_batch.h"

#include "utils/log.h"
#include "c_api.h"

// 
void ui_render_batch_t::init()
{
    // 
    m_vao_quads.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },    // pad
    });
    size_t vertices_max_size = SYN_UI_BATCH_MAX_QUADS * sizeof(ui_render_vertex_t) * 4;
    size_t indicies_max_size = SYN_UI_BATCH_MAX_QUADS * sizeof(uint32_t) * 6;

    m_vao_quads.create_empty_vertices(vertices_max_size);
    m_vao_quads.create_empty_indices(indicies_max_size);

    // 
    m_vao_lines.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },    // pad
    });
    vertices_max_size = SYN_UI_BATCH_MAX_LINES * sizeof(ui_render_vertex_t) * 8;
    indicies_max_size = SYN_UI_BATCH_MAX_LINES * sizeof(uint32_t) * 9;

    m_vao_lines.create_empty_vertices(vertices_max_size);
    m_vao_lines.create_empty_indices(indicies_max_size);
        
    m_shader_handle = shader_lib.load_from_file("ui_window_base_shader", 
        "../assets/shaders/ui_window_base.glsl");

    m_is_initialized = true;
    
}

// 
void ui_render_batch_t::shutdown()
{
    m_vao_quads.destroy();
    
}

// 
void ui_render_batch_t::begin_batch()
{
    // no memset m_quad_vertices to 0, just start over
    m_quad_vertex_count = 0;
    m_quad_index_count = 0;

    m_line_vertex_count = 0;
    m_line_index_count = 0;
    
}

// 
void ui_render_batch_t::add_quad(const glm::vec2 &_position,
                               const glm::vec2 &_size,
                               const glm::vec4 &_color,
                               float _depth)
{
    if (m_quad_vertex_count + 4 >= SYN_UI_BATCH_MAX_QUADS * 4 ||
        m_quad_index_count + 6 >= SYN_UI_BATCH_MAX_QUADS * 6) 
    {
        SYN_WARNING("UI quad batch full, flushing early.\n");
        end_batch();
        begin_batch();
    }

    uint32_t vertex_offset = m_quad_vertex_count;
    uint32_t index_offset = m_quad_index_count;

    m_quad_vertices[vertex_offset + 0] = ui_render_vertex_t({ _position.x,           _position.y           }, _color, _depth);
    m_quad_vertices[vertex_offset + 1] = ui_render_vertex_t({ _position.x + _size.x, _position.y           }, _color, _depth);
    m_quad_vertices[vertex_offset + 2] = ui_render_vertex_t({ _position.x + _size.x, _position.y + _size.y }, _color, _depth);
    m_quad_vertices[vertex_offset + 3] = ui_render_vertex_t({ _position.x,           _position.y + _size.y }, _color, _depth);

    m_quad_indices[index_offset + 0] = vertex_offset + 0;
    m_quad_indices[index_offset + 1] = vertex_offset + 3;
    m_quad_indices[index_offset + 2] = vertex_offset + 2;
    m_quad_indices[index_offset + 3] = vertex_offset + 2;
    m_quad_indices[index_offset + 4] = vertex_offset + 1;
    m_quad_indices[index_offset + 5] = vertex_offset + 0;

    m_quad_vertex_count += 4;
    m_quad_index_count += 6;    
}

// 
void ui_render_batch_t::add_line_strip(ui_render_vertex_t *_vertices, size_t _vertex_count)
{
    uint32_t vertex_offset = m_line_vertex_count;
    uint32_t index_offset = m_line_index_count;

    for (size_t i = 0; i < _vertex_count; i++) {
        m_line_vertices[vertex_offset + i] = _vertices[i];
        m_line_indices[index_offset + i] = vertex_offset + i;
    }
    m_line_vertex_count += _vertex_count;
    m_line_index_count += _vertex_count;
    
    m_line_indices[m_line_index_count++] = 0xFFFFFFFF;
    
}

// 
void ui_render_batch_t::end_batch()
{
    if (!m_quad_vertex_count && !m_line_vertex_count &&
        !m_quad_index_count && m_line_index_count) return;
    
    shader_t *shader;
    shader = shader_lib.get_shader(m_shader_handle);
    if (!shader) return;

    glm::mat4 proj = glm::ortho(0.0f, root_window.get_fwidth(), 
                                root_window.get_fheight(), 0.0f, 
                                window_manager.m_zfar, window_manager.m_znear);
    shader->enable();
    shader->set_matrix_4fv("u_projection", proj);

    if (m_quad_vertex_count > 0 && m_quad_index_count > 0)
    {
        m_vao_quads.bind();
        m_vao_quads.update_vertices(m_quad_vertices, m_quad_vertex_count * sizeof(ui_render_vertex_t));
        m_vao_quads.update_indices(m_quad_indices, m_quad_index_count * sizeof(uint32_t));
        glDrawElements(GL_TRIANGLES, m_quad_index_count, GL_UNSIGNED_INT, 0);
        m_vao_quads.unbind();
    }

    // 
    if (m_line_vertex_count > 0 && m_line_index_count > 0) {
        m_vao_lines.bind();
        m_vao_lines.update_vertices(m_line_vertices, m_line_vertex_count * sizeof(ui_render_vertex_t));
        m_vao_lines.update_indices(m_line_indices, m_line_index_count * sizeof(uint32_t));
        glDrawElements(GL_LINE_STRIP, m_line_index_count, GL_UNSIGNED_INT, 0);
        m_vao_lines.unbind();
    }

    shader->disable();
    
}

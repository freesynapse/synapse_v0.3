
#include "renderer/UI/window/ui_quad_batch.h"

#include "utils/log.h"
#include "c_api.h"

// 
void ui_quad_batch_t::init()
{
    m_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },    // pad
    });
    size_t vertices_max_size = SYN_UI_BATCH_MAX_QUADS * sizeof(ui_quad_vertex_t) * 4;
    size_t indicies_max_size = SYN_UI_BATCH_MAX_QUADS * sizeof(uint32_t) * 6;

    m_vao.create_empty_vertices(vertices_max_size);
    m_vao.create_empty_indices(indicies_max_size);

    m_shader_handle = shader_lib.load_from_file("ui_window_base_shader", 
        "../assets/shaders/ui_window_base.glsl");

    m_is_initialized = true;
    
}

// 
void ui_quad_batch_t::shutdown()
{
    m_vao.destroy();
    
}

// 
void ui_quad_batch_t::begin_batch()
{
    // no memset m_vertices to 0, just start over
    m_vertex_count = 0;
    m_index_count = 0;
    
}

// 
void ui_quad_batch_t::add_quad(const glm::vec2 &_position,
                               const glm::vec2 &_size,
                               const glm::vec4 &_color,
                               float _depth)
{
    if (m_vertex_count + 4 >= SYN_UI_BATCH_MAX_QUADS * 4 ||
        m_index_count + 6 >= SYN_UI_BATCH_MAX_QUADS * 6) 
    {
        SYN_WARNING("UI quad batch full, flushing early.\n");
        end_batch();
        begin_batch();
    }

    uint32_t vertex_offset = m_vertex_count;
    uint32_t index_offset = m_index_count;

    m_vertices[vertex_offset + 0] = ui_quad_vertex_t({ _position.x,           _position.y           }, _color, _depth);
    m_vertices[vertex_offset + 1] = ui_quad_vertex_t({ _position.x + _size.x, _position.y           }, _color, _depth);
    m_vertices[vertex_offset + 2] = ui_quad_vertex_t({ _position.x + _size.x, _position.y + _size.y }, _color, _depth);
    m_vertices[vertex_offset + 3] = ui_quad_vertex_t({ _position.x,           _position.y + _size.y }, _color, _depth);

    m_indices[index_offset + 0] = vertex_offset + 0;
    m_indices[index_offset + 1] = vertex_offset + 1;
    m_indices[index_offset + 2] = vertex_offset + 2;
    m_indices[index_offset + 3] = vertex_offset + 2;
    m_indices[index_offset + 4] = vertex_offset + 3;
    m_indices[index_offset + 5] = vertex_offset + 0;

    m_vertex_count += 4;
    m_index_count += 6;    
}

// 
void ui_quad_batch_t::end_batch(bool _ui_shader_already_bound)
{
    if (m_vertex_count == 0 || m_index_count == 0) return;

    shader_t *shader;
    if (!_ui_shader_already_bound) {
        shader = shader_lib.get_shader(m_shader_handle);
        if (!shader) return;
    
        glm::mat4 proj = glm::ortho(0.0f, root_window.get_fwidth(), 
                                    root_window.get_fheight(), 0.0f, 
                                    window_manager.m_zfar, window_manager.m_znear);
        shader->enable();
        shader->set_matrix_4fv("u_projection", proj);
    }
    
    m_vao.bind();
    m_vao.update_vertices(m_vertices, sizeof(ui_quad_vertex_t) * m_vertex_count);
    m_vao.update_indices(m_indices, sizeof(uint32_t) * m_index_count);
    glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);
    m_vao.unbind();

    if (!_ui_shader_already_bound) shader->disable();
    
}

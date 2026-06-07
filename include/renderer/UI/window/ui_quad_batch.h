#ifndef __UI_QUAD_BATCH_H
#define __UI_QUAD_BATCH_H

#include <vector>
#include <glm/glm.hpp>
#include "renderer/buffers/vertex_array.h"
#include "renderer/shader/shader_types.h"

// 
#define SYN_UI_BATCH_MAX_QUADS  1024

// 
struct ui_quad_vertex_t {
    glm::vec2 pos;
    glm::vec4 color;
    float depth;
    float pad;

    ui_quad_vertex_t(const glm::vec2 &_position, const glm::vec4 &_color, float _depth) :
        pos(_position), color(_color), depth(_depth), pad(0.0f) 
    {}
    ui_quad_vertex_t() {};
    
};

// 
class ui_quad_batch_t
{
public:
    ui_quad_batch_t() = default;
    ~ui_quad_batch_t() = default;
    
    void init();
    void shutdown();

    void begin_batch();
    void add_quad(const glm::vec2 &_position,
                  const glm::vec2 &_size,
                  const glm::vec4 &_color,
                  float _depth);
    void end_batch(bool _ui_shader_already_bound=false);

    bool is_initalized() { return m_is_initialized; }
    
private:
    ui_quad_vertex_t m_vertices[SYN_UI_BATCH_MAX_QUADS * 4];
    size_t m_vertex_count = 0;

    uint32_t m_indices[SYN_UI_BATCH_MAX_QUADS * 6];
    size_t m_index_count = 0;
    
    vertex_array_t m_vao;
    shader_handle_t m_shader_handle;

    bool m_is_initialized = false;
    
};



#endif // __UI_QUAD_BATCH_H

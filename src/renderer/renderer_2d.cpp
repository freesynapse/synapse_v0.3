
#include "renderer/renderer_2d.h"

#include "utils/log.h"

#include "c_api.h"


// 
void renderer_2d_t::init()
{
    init_ui_quad();
    
}

// 
void renderer_2d_t::shutdown()
{
    
}

// 
std::array<vertex_data_2d_t, 4> renderer_2d_t::quad_t_to_vertex_2d_t(const quad_2d_t &_q)
{
    glm::vec2 v_[4] = {
        {-0.5f, -0.5f},
        { 0.5f, -0.5f},
        { 0.5f,  0.5f},
        {-0.5f,  0.5f},
    };

    float c = cosf(_q.transform.rotation);
    float s = sinf(_q.transform.rotation);

    std::array<vertex_data_2d_t, 4> v;
    for (size_t i = 0; i < 4; i++) {
        // scale
        glm::vec2 p = v_[i] * _q.transform.scale;
        // rotate
        p = { 
            p.x * c - p.y * s, 
            p.x * s + p.y * c
        };
        // translate
        p += _q.transform.position;

        v[i] = {
            p,
            v_[i] + 0.5f,   // uv [0..1]
            _q.color,
            _q.tex_index,
            _q.depth
        };
    }

    return v;
}

//
void renderer_2d_t::init_ui_quad()
{
  // since glm::ortho, we need a different winding order
  glm::vec2 vertices[] = {
      { 1.0f, 1.0f },
      { 1.0f, 0.0f },
      { 0.0f, 0.0f },
      { 0.0f, 1.0f },
  };

  vertex_array_t vao;
  vao.set_buffer_layout({ 
      { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 }
  });

  // uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

  vao.create(vertices, sizeof(vertices) / sizeof(glm::vec2)); //, indices, 6);
  m_ui_quad_vao = vao;

  m_ui_shader_handle = shader_lib.load_from_file("ui_progress_shader", "../assets/shaders/ui_progress_shader.glsl");

  SYN_INFO("asset loader gui created.\n");
  
}

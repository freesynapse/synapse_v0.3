
#include "renderer/UI/window/window.h"
#include "utils/log.h"

#include "c_api.h"


// 
void window_t::init()
{
    struct vertex {
        glm::vec2 pos;
        glm::vec4 color;
        float depth;
        float pad;
    };

    glm::vec2 p = position;
    glm::vec2 s = size;
    float tb_h = title_bar_height;

    glm::vec4 tb_color = m_is_focused ? title_bar_color_focused : title_bar_color;
    
    vertex v[] = {
        { { p.x,       p.y         }, tb_color, depth - 0.02f, 0.0f },
        { { p.x + s.x, p.y         }, tb_color, depth - 0.02f, 0.0f },
        { { p.x + s.x, p.y + tb_h  }, tb_color, depth - 0.02f, 0.0f },
        { { p.x,       p.y + tb_h  }, tb_color, depth - 0.02f, 0.0f },
        
        { { p.x,       p.y + tb_h }, bg_color, depth, 0.0f },
        { { p.x + s.x, p.y + tb_h }, bg_color, depth, 0.0f },
        { { p.x + s.x, p.y + s.y  }, bg_color, depth, 0.0f },
        { { p.x,       p.y + s.y  }, bg_color, depth, 0.0f },
    };

    uint32_t indices[] = { 
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
    };

    m_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },
    });
    m_vao.create(v, sizeof(v) / sizeof(v[0]), indices, sizeof(indices) / sizeof(uint32_t));
    
    m_is_visible = true;
    m_is_active = true;

    SYN_INFO("window '%s' created (depth=%.2f).\n", name.c_str(), depth);
    
}

// 
void window_t::destroy()
{
    if (m_is_active) {
        m_vao.destroy();
    }
}

// 
void window_t::draw()
{
    m_vao.bind();
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    m_vao.unbind();
    
    float tx = position.x + 10.0f;
    float ty = position.y + (title_bar_height - font.get_font_height()  * 0.5f);
    font.set_color(fg_color);
    font.set_depth(depth - 0.03f);
    font.render_text(tx, ty, "%s", name.c_str());

}

// 
bool window_t::contains_point(float _x, float _y)
{
    return (_x >= position.x && _x <= position.x + size.x &&
            _y >= position.y && _y <= position.y + size.y);
}

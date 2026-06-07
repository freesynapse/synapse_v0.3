
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

    // set up the body and the title of the window
    glm::vec2 p = position;
    glm::vec2 s = size;
    float tb_h = title_bar_height;

    glm::vec4 tb_color = m_is_focused ? title_bar_color_focused : title_bar_color;
    
    vertex v[] = {
        { { p.x,       p.y         }, tb_color, depth - 0.005f, 0.0f },
        { { p.x + s.x, p.y         }, tb_color, depth - 0.005f, 0.0f },
        { { p.x + s.x, p.y + tb_h  }, tb_color, depth - 0.005f, 0.0f },
        { { p.x,       p.y + tb_h  }, tb_color, depth - 0.005f, 0.0f },
        
        { { p.x,       p.y + tb_h }, bg_color, depth, 0.0f },
        { { p.x + s.x, p.y + tb_h }, bg_color, depth, 0.0f },
        { { p.x + s.x, p.y + s.y  }, bg_color, depth, 0.0f },
        { { p.x,       p.y + s.y  }, bg_color, depth, 0.0f },
    };

    uint32_t indices[] = { 
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
    };

    m_vao_body.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },
    });
    m_vao_body.create(v, sizeof(v) / sizeof(v[0]), indices, sizeof(indices) / sizeof(uint32_t));

    // setup window outline
    vertex l[] = {
        { { p.x,       p.y + tb_h }, outline_color, depth - 0.03f, 0.0f },
        { { p.x + s.x, p.y + tb_h }, outline_color, depth - 0.03f, 0.0f },
        { { p.x + s.x, p.y + s.y  }, outline_color, depth - 0.03f, 0.0f },
        { { p.x,       p.y + s.y  }, outline_color, depth - 0.03f, 0.0f },
        { { p.x,       p.y        }, outline_color, depth - 0.03f, 0.0f },
    };

    m_vao_outline.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },
    });
    m_vao_outline.create(l, sizeof(l) / sizeof(l[0]));

    
    
    // default flags
    m_is_visible = true;
    m_is_active = true;

}

// 
void window_t::destroy()
{
    if (m_is_active) {
        m_vao_body.destroy();
        m_vao_outline.destroy();
    }
}

// 
void window_t::draw()
{
    m_vao_body.bind();
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    m_vao_body.unbind();

    m_vao_outline.bind();
    api.set_line_width(1.0f);
    glDrawArrays(GL_LINE_STRIP, 0, 5);
    m_vao_outline.unbind();
    
    float tx = position.x + 10.0f;
    float ty = position.y + (title_bar_height - font.get_font_height()  * 0.5f);
    font.set_color(fg_color);
    float font_depth = (depth - 0.01f) / window_manager.m_zfar;
    font.set_depth(font_depth);
    font.render_text(tx, ty, "%s", name.c_str());

}

// 
bool window_t::is_point_in_window(const glm::vec2 &_p)
{
    return (_p.x >= position.x && _p.x <= position.x + size.x &&
            _p.y >= position.y && _p.y <= position.y + size.y);
}

// 
bool window_t::is_point_in_title_bar(const glm::vec2 &_p)
{
    return (_p.x >= position.x && _p.x <= position.x + size.x &&
            _p.y >= position.y && _p.y <= position.y + title_bar_height);
}

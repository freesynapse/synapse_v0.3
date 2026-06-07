
#include <vector>

#include "renderer/UI/window/window_manager.h"
#include "utils/log.h"

#include "c_api.h"

//
static void __window_on_mouse_button_callback(const event_t &_e) { window_manager.on_mouse_button_event(_e); }
static void __window_on_mouse_move_callback(const event_t &_e) { window_manager.on_mouse_move_event(_e); }

//
void window_manager_t::init()
{
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        m_pool[i] = window_t();
    }
    m_active_count = 0;
    
    m_window_shader_handle = shader_lib.load_from_file(
        "ui_window_base_shader", "../assets/shaders/ui_window_base.glsl");
    
    //
    events.register_callback(event_type_t::INPUT_MOUSE_BUTTON, __window_on_mouse_button_callback);
    events.register_callback(event_type_t::INPUT_MOUSE_MOVE, __window_on_mouse_move_callback);

    if (!ui_quad_batch.is_initalized()) {
        ui_quad_batch.init();
    }
}

//
void window_manager_t::shutdown()
{
    for (uint32_t i = 0; i < m_active_count; i++) {
        if (m_pool[i].is_active()) {
            m_pool[i].destroy();
        }
    }
}
    
//
void window_manager_t::on_mouse_button_event(const event_t &_e)
{
    int action = _e.as.mouse_button.action;
    int button = _e.as.mouse_button.button;
    glm::vec2 pos = _e.as.mouse_button.pos;
    
    if (button != SYN_MOUSE_BUTTON_LEFT) return;
    
    window_handle_t clicked = get_window_at_pos(pos);
    if (action == SYN_MOUSE_BUTTON_PRESSED) {
    
        if (clicked.id != 0) {
            window_t *win = get_window(clicked);

            // first check for widget
            widget_t *clicked_widget = win->get_widget_at_pos(pos);
            if (clicked_widget) {
                clicked_widget->on_click();
                return;
            }
            
            //
            if (win->m_is_movable &&
                pos.y >= win->position.y && 
                pos.y <= win->position.y + win->title_bar_height) 
            {
                m_is_dragging = true;
                m_drag_window_handle = clicked;
                m_drag_offset = pos - win->position;
                init_drag_vao();
            }
            
            if (clicked.id != m_focused_window.id) {
                set_focused_window(clicked);
            }
        } else {
            set_focused_window({0});
        }
    
    } else if (action == SYN_MOUSE_BUTTON_RELEASED) {
        if (m_is_dragging) {
            window_t *win = get_window(m_drag_window_handle);
            win->position = pos - m_drag_offset;
            win->destroy();
            win->init();
        
            m_is_dragging = false;
            m_drag_window_handle = {0};
            m_drag_vao.destroy();
        }
    }
}

//
void window_manager_t::on_mouse_move_event(const event_t &_e)
{
    if (!m_is_dragging || m_drag_window_handle.id == 0) return;

    if (m_drag_vao.m_array_id != 0) {
        update_drag_vao(_e.as.mouse_move.pos);
    }

    // moving the window, destroy()ing and init()ing it every mouse move
    // 
    // glm::vec2 pos = _e.as.mouse_move.pos;
    
    // window_t *win = get_window(m_drag_window_handle);
    // if (!win) return;
    
    // glm::vec2 new_pos = pos - m_drag_offset;
    // glm::clamp(new_pos.x, 0.0f, root_window.get_fwidth() - win->size.x);
    // glm::clamp(new_pos.y, 0.0f, root_window.get_fheight() - win->size.y);
    
    // win->position = new_pos;
    
    // win->destroy();
    // win->init();
}
//
window_handle_t window_manager_t::add_window(window_t &_window)
{
    // search for duplicates
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        if (m_pool[i].is_active() && _window.name == m_pool[i].name) {
            return {i + 1};
        }
    }
    
    // check overflow
    if (m_active_count >= SYN_MAX_WINDOW_COUNT) {
        SYN_ERROR("number of windows >= SYN_MAX_WINDOW_COUNT.\n");
        return {0};
    }
    
    uint32_t slot = m_active_count;
    m_pool[slot] = _window;
    window_handle_t handle = {slot + 1};
    
    window_t *win = &m_pool[slot];
    
    if (win->depth == 0.0f) {
        win->depth = m_next_depth;
        m_next_depth -= m_ddepth_per_layer;
    } else if (win->depth <= m_next_depth) {
        m_next_depth = win->depth - m_ddepth_per_layer;
    }
    
    win->init();
    win->m_is_active = true;
    win->m_is_visible = true;
    
    // set to focused?
    if (win->is_focused()) {
        set_focused_window(handle);
    }
    
    //
    m_active_count++;
    
    return handle;
}

//
window_handle_t window_manager_t::add_window(const window_desc_t &_desc)
{
    window_t window;
    window.position = _desc.position;
    window.size = _desc.size;
    
    return add_window(window);
}

//
window_t *window_manager_t::get_window(const window_handle_t &_handle)
{
    uint32_t idx = _handle.id - 1;
    if (_handle.id == 0 || idx >= SYN_MAX_WINDOW_COUNT) {
        SYN_WARNING("invalid window_handle_t: id = %d.\n", _handle.id);
        return nullptr;
    }
    
    return &m_pool[idx];
}

//
window_handle_t window_manager_t::get_window_at_pos(const glm::vec2 _pos)
{
    // serach from highest depth
    window_handle_t top_window = {0};
    float highest_depth = m_zfar;
    
    for (uint32_t i = 0; i < m_active_count; i++) {
        window_t *win = &m_pool[i];
        if (!win->is_active() || !win->is_visible()) continue;
    
        if (win->is_point_in_window(_pos)) {
            if (win->depth <= highest_depth) {
                highest_depth = win->depth;
                top_window = {i + 1};
            }
        }
    }
    
    return top_window;
}

//
void window_manager_t::set_focused_window(window_handle_t _handle)
{
    // unfocus previous
    if (m_focused_window.id != 0) {
        window_t *prev = get_window(m_focused_window);
        if (prev) {
            prev->set_focused(false);
            prev->destroy();
            prev->init();
        }
    }

    //  focus new window and bring to front
    if (_handle.id != 0) {
        window_t *win = get_window(_handle);
        if (win) {
            win->set_focused(true);
            win->depth = m_next_depth;
            m_next_depth -= 0.05;
        
            win->destroy();
            win->init();
        
            if (m_next_depth < -99.0f) {
                reorganize_depths();
            }
        }
    }
    
    m_focused_window = _handle;
}

//
void window_manager_t::draw_windows()
{
    // glm::ivec2 dims = root_window.window_dims();
    // m_projection = glm::ortho(0.0f, (float)dims.x, (float)dims.y, 0.0f, m_zfar, m_znear);
    
    // shader_t *shader = shader_lib.get_shader(m_window_shader_handle);
    // if (!shader) {
            // SYN_FATAL_ERROR("invalid window manager shader handle.\n");
            // return;
    // }
    
    // shader->enable();
    // shader->set_matrix_4fv("u_projection", m_projection);
    
    api.set_depth_testing(true);
    api.set_depth_func(GL_LEQUAL);
    api.set_depth_mask(GL_TRUE);

    ui_quad_batch.begin_batch();
    
    //
    for (uint32_t i = 0; i < m_active_count; i++) {
        window_t *win = &m_pool[i];
        if (win->is_active() && win->is_visible()) {
            win->draw();
        }
    }

    bool ui_shader_bound = false;
    if (m_drag_vao.m_array_id != 0) {
        glm::ivec2 dims = root_window.window_dims();
        m_projection = glm::ortho(0.0f, (float)dims.x, (float)dims.y, 0.0f, m_zfar, m_znear);
        
        shader_t *shader = shader_lib.get_shader(m_window_shader_handle);
        if (!shader) {
            SYN_FATAL_ERROR("invalid window manager shader handle.\n");
            return;
        }
        
        shader->enable();
        shader->set_matrix_4fv("u_projection", m_projection);
        
        m_drag_vao.bind();
        glDrawArrays(GL_LINE_STRIP, 0, 8);
        m_drag_vao.unbind();

        // shader enabling and projection matrix preset, skip on batch flushing
        ui_shader_bound = true;
    }

    ui_quad_batch.end_batch(ui_shader_bound);
    font.end_render_block(true);
    
}

//
void window_manager_t::reorganize_depths()
{
    struct window_depth_pair {
        uint32_t index;
        float depth;
    };
    
    std::vector<window_depth_pair> active_windows;
    
    for (uint32_t i = 0; i < m_active_count; i++) {
        if (m_pool[i].is_active()) {
        active_windows.push_back({i, m_pool[i].depth});
        }
    }
    
    // sort by depth
    std::sort(active_windows.begin(), active_windows.end(),
                [](const window_depth_pair &a, const window_depth_pair &b) {
                return a.depth < b.depth;
                });
    
    // reassign depths starting from zfar
    float new_depth = m_zfar - 1.0f - m_ddepth_per_layer * m_active_count;
    float closest_depth = new_depth;
    for (auto &pair : active_windows) {
        window_t *win = &m_pool[pair.index];
    
        // only update if depth change
        if (std::abs(win->depth - new_depth) > 0.01f) {
            win->depth = new_depth;
            win->destroy();
            win->init();
        }
        new_depth += m_ddepth_per_layer;
    }
    
    m_next_depth = closest_depth - m_ddepth_per_layer;
}

//
void window_manager_t::init_drag_vao()
{
    if (m_drag_window_handle.id == 0) return;
    window_t *win = get_window(m_drag_window_handle);
    if (!win) return;
    
    struct vertex {
        glm::vec2 pos;
        glm::vec4 color;
        float depth;
        float pad;
    };

    // set up the body and the title of the window
    glm::vec2 p = win->position;
    glm::vec2 s = win->size;
    float tb_h = win->title_bar_height;
    glm::vec4 color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    float d = m_znear;
    
    vertex v[] = {
        { { p.x,       p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y        }, color, d, 0.0f },
        { { p.x + s.x, p.y        }, color, d, 0.0f },
        { { p.x + s.x, p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y + s.y  }, color, d, 0.0f },
        { { p.x + s.x, p.y + s.y  }, color, d, 0.0f },
        { { p.x + s.x, p.y + tb_h }, color, d, 0.0f },
    };

    m_drag_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 },
        { VERTEX_ATTRIB_LOCATION_DEPTH, shader_data_type_t::FLOAT },
        { VERTEX_ATTRIB_LOCATION_DEPTH + 1, shader_data_type_t::FLOAT },
    });
    m_drag_vao.create(v, sizeof(v) / sizeof(v[0]));

}

//
void window_manager_t::update_drag_vao(const glm::vec2 &_pos)
{
    window_t *win = get_window(m_drag_window_handle);
    if (!win) return;

    // update vertices
    struct vertex {
        glm::vec2 pos;
        glm::vec4 color;
        float depth;
        float pad;
    };

    glm::vec2 p = _pos - m_drag_offset;
    glm::vec2 s = win->size;
    float tb_h = win->title_bar_height;
    glm::vec4 color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    float d = m_znear;

    vertex v[] = {
        { { p.x,       p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y        }, color, d, 0.0f },
        { { p.x + s.x, p.y        }, color, d, 0.0f },
        { { p.x + s.x, p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y + tb_h }, color, d, 0.0f },
        { { p.x,       p.y + s.y  }, color, d, 0.0f },
        { { p.x + s.x, p.y + s.y  }, color, d, 0.0f },
        { { p.x + s.x, p.y + tb_h }, color, d, 0.0f },
    };

    m_drag_vao.update_vertices(v, sizeof(v), 0);
    
}

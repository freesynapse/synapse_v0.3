
#include <vector>

#include "renderer/UI/window/window_manager.h"
#include "utils/log.h"

#include "c_api.h"

// 
static void __window_on_mouse_button_callback(const event_t &_e) { window_manager.on_mouse_button_event(_e); }

// 
void window_manager_t::init()
{
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        m_pool[i] = window_t();
    }
    m_active_count = 0;

    m_window_shader_handle = shader_lib.load_from_file("ui_window_shader", 
        "../assets/shaders/ui_window.glsl");

    // 
    events.register_callback(event_type_t::INPUT_MOUSE_BUTTON, __window_on_mouse_button_callback);
    
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
window_handle_t window_manager_t::add_window(window_t &_window)
{
    // search for duplicates
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        if (m_pool[i].is_active() && _window.name == m_pool[i].name) {
            return { i + 1 };
        }
    }

    // check overflow
    if (m_active_count >= SYN_MAX_WINDOW_COUNT) {
        SYN_ERROR("number of windows >= SYN_MAX_WINDOW_COUNT.\n");
        return { 0 };
    }

    uint32_t slot = m_active_count;
    m_pool[slot] = _window;
    window_handle_t handle = { slot + 1 };

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
window_handle_t window_manager_t::get_window_at_pos(float _x, float _y)
{
    // serach from highest depth
    window_handle_t top_window = { 0 };
    float highest_depth = 1.0f;

    for (uint32_t i = 0; i < m_active_count; i++) {
        window_t *win = &m_pool[i];
        if (!win->is_active() || !win->is_visible()) continue;

        if (win->contains_point(_x, _y)) {
            if (win->depth < highest_depth) {
                highest_depth = win->depth;
                top_window = { i + 1 };
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
void window_manager_t::on_mouse_button_event(const event_t &_e)
{
    int action = _e.as.mouse_button.action;
    SYN_INFO("mouse clicked... %d\n", action);
    
}

// 
void window_manager_t::draw_windows()
{
    glm::ivec2 dims = root_window.window_dims();
    // glm::mat4 proj = glm::ortho(0.0f, (float)dims.x, (float)dims.y, 0.0f, -1.0f, 1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)dims.x, (float)dims.y, 0.0f, m_zfar, m_znear);

    shader_t *shader = shader_lib.get_shader(m_window_shader_handle);
    if (!shader) {
        SYN_FATAL_ERROR("invalid window manager shader handle.\n");
        return;
    }

    shader->enable();
    shader->set_matrix_4fv("u_projection", proj);

    // 
    for (uint32_t i = 0; i < m_active_count; i++) {
        window_t *win = &m_pool[i];
        if (win->is_active() && win->is_visible()) {
            win->draw();
        }
    }
    
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
            active_windows.push_back({ i, m_pool[i].depth });
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
            printf("window %s new depth = %f\n", win->name.c_str(), win->depth);
        }

        new_depth += m_ddepth_per_layer;
    }

    m_next_depth = closest_depth - m_ddepth_per_layer;
    
}
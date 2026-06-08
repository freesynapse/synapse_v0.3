
#include <vector>

#include "renderer/UI/window/window_manager.h"
#include "utils/log.h"

#include "c_api.h"

//
static void __window_on_mouse_button_callback(const event_t &_e) { window_manager.on_mouse_button_event(_e); }
static void __window_on_mouse_move_callback(const event_t &_e) { window_manager.on_mouse_move_event(_e); }
static void __window_on_ui_window_close_callback(const event_t &_e) { window_manager.on_ui_window_close_event(_e); }
//
void window_manager_t::init()
{
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        m_pool[i] = window_t();
    }
    m_active_count = 0;
    
    m_window_shader_handle = shader_lib.load_from_file("ui_window_base_shader", "../assets/shaders/ui_window_base.glsl");
    m_tex_quad_shader_handle = shader_lib.load_from_file("ui_tex_quad_shader", "../assets/shaders/ui_quad_tex.glsl");

    // 
    glm::vec2 vs[] = { 
        { 0.0f, 0.0f },
        { 0.0f, 1.0f }, 
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
    };
    uint32_t is[] = { 0, 1, 2, 2, 3, 0 };
    m_tex_quad_vao.set_buffer_layout({ { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 } });
    m_tex_quad_vao.create(vs, 4, is, 6);
    
    //
    events.register_callback(event_type_t::INPUT_MOUSE_BUTTON, __window_on_mouse_button_callback);
    events.register_callback(event_type_t::INPUT_MOUSE_MOVE, __window_on_mouse_move_callback);
    events.register_callback(event_type_t::UI_WINDOW_CLOSE, __window_on_ui_window_close_callback);
    
    if (!ui_batch_renderer.is_initalized()) {
        ui_batch_renderer.init();
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

    m_tex_quad_vao.destroy();
    
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
    
        if (clicked.id > 0) {
            window_t *win = &m_pool[clicked.id - 1];

            // first check for widget
            widget_t *clicked_widget = win->get_widget_at_pos(pos);
            if (clicked_widget) {
                clicked_widget->on_click();
                return;
            }

            // check for resize
            resize_handle_t resize_handle = win->get_resize_handle_at_pos(pos);
            if (win->m_is_resizable && resize_handle != resize_handle_t::NONE) {
                m_is_resizing = true;
                m_resize_window_handle = clicked;
                m_resize_handle = resize_handle;
                m_resize_start_pos = pos;
                m_resize_start_size = win->size;
                m_resize_start_window_pos = win->position;
                
                set_focused_window(clicked);
                
                return;
            }
            
            // check for moving
            if (win->m_is_movable &&
                pos.y >= win->position.y && 
                pos.y <= win->position.y + win->title_bar_height) 
            {
                m_is_dragging = true;
                m_drag_window_handle = clicked;
                m_mouse_pos = pos;
                m_drag_offset = pos - win->position;

                set_focused_window(clicked);
            }

            // select clicked window
            if (clicked.id != m_focused_window.id) {
                set_focused_window(clicked);
                return;
            }
        } else {
            set_focused_window({ 0 });
        }
    
    } else if (action == SYN_MOUSE_BUTTON_RELEASED) {
        if (m_is_dragging) {
            window_t *win = get_window(m_drag_window_handle);
            win->position = pos - m_drag_offset;
            m_is_dragging = false;
            m_drag_window_handle = { 0 };
        }

        if (m_is_resizing) {
            m_is_resizing = false;
            m_resize_window_handle = { 0 };
            m_resize_handle = resize_handle_t::NONE;
        }
    }
}

//
void window_manager_t::on_mouse_move_event(const event_t &_e)
{
    m_mouse_pos = _e.as.mouse_move.pos;

    //  hovering
    m_hovered_window = get_window_at_pos(m_mouse_pos);
    if (m_hovered_window.id > 0) {
        window_t *win = &m_pool[m_hovered_window.id - 1];
        resize_handle_t handle = win->get_resize_handle_at_pos(m_mouse_pos);

        switch (handle) {
            case resize_handle_t::LEFT:
            case resize_handle_t::RIGHT:
                root_window.set_cursor(GLFW_RESIZE_EW_CURSOR);
                break;

            case resize_handle_t::TOP:
            case resize_handle_t::BOTTOM:
                root_window.set_cursor(GLFW_RESIZE_NS_CURSOR);
                break;

            case resize_handle_t::TOP_LEFT:
            case resize_handle_t::BOTTOM_RIGHT:
                root_window.set_cursor(GLFW_RESIZE_NWSE_CURSOR);
                break;

            case resize_handle_t::TOP_RIGHT:
            case resize_handle_t::BOTTOM_LEFT:
                root_window.set_cursor(GLFW_RESIZE_NESW_CURSOR);
                break;

            default:
                root_window.set_cursor(GLFW_ARROW_CURSOR);
                break;
        }
    }
    else {
        root_window.set_cursor(GLFW_ARROW_CURSOR);
    }
    
    // resizing window
    if (m_is_resizing && m_resize_window_handle.id > 0) {
        window_t *win = &m_pool[m_resize_window_handle.id - 1];
        if (win->m_is_resizable) {
            glm::vec2 delta = m_mouse_pos - m_resize_start_pos;
            glm::vec2 new_size = m_resize_start_size;
            glm::vec2 new_pos = m_resize_start_window_pos;
    
            switch (m_resize_handle) {
                case resize_handle_t::RIGHT: {
                    new_size.x = m_resize_start_size.x + delta.x;
                    break;
                }
                case resize_handle_t::BOTTOM: {
                    new_size.y = m_resize_start_size.y + delta.y;
                    break;
                }
                case resize_handle_t::LEFT: {
                    new_size.x = m_resize_start_size.x - delta.x;
                    new_pos.x = m_resize_start_window_pos.x + delta.x;
                    break;
                }
                case resize_handle_t::TOP: {
                    new_size.y = m_resize_start_size.y - delta.y;
                    new_pos.y = m_resize_start_window_pos.y + delta.y;
                    break;
                }
                case resize_handle_t::BOTTOM_RIGHT: {
                    new_size.x = m_resize_start_size.x + delta.x;
                    new_size.y = m_resize_start_size.y + delta.y;
                    break;
                }
                case resize_handle_t::BOTTOM_LEFT: {
                    new_size.x = m_resize_start_size.x - delta.x;
                    new_size.y = m_resize_start_size.y + delta.y;
                    new_pos.x = m_resize_start_window_pos.x + delta.x;
                    break;
                }
                case resize_handle_t::TOP_RIGHT: {
                    new_size.x = m_resize_start_size.x + delta.x;
                    new_size.y = m_resize_start_size.y - delta.y;
                    new_pos.y = m_resize_start_window_pos.y + delta.y;
                    break;
                }
                case resize_handle_t::TOP_LEFT: {
                    new_size.x = m_resize_start_size.x - delta.x;
                    new_size.y = m_resize_start_size.y - delta.y;
                    new_pos.x = m_resize_start_window_pos.x + delta.x;
                    new_pos.y = m_resize_start_window_pos.y + delta.y;
                    break;
                }
    
                default: break;
            }
            new_size.x = glm::clamp(new_size.x, win->min_size.x, win->max_size.x);
            new_size.y = glm::clamp(new_size.y, win->min_size.y, win->max_size.y);
    
            win->size = new_size;
            win->position = new_pos;

            if (win->m_has_framebuffer) {
                win->resize_framebuffer();
            }
            
            return;
        }
    }

    // moving windows -- the ui_batch_renderer takes care of the rendering
    if (m_is_dragging && m_drag_window_handle.id > 0) {
        window_t *win = &m_pool[m_drag_window_handle.id - 1];
        win->position = m_mouse_pos - m_drag_offset;
    }
    
    // widgets
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (!win->m_is_active || !win->m_is_visible) continue;

        // reset all hover flags
        for (uint32_t j = 0; j < win->m_widget_count; j++) {
            win->m_widgets[j].is_hovered = false;
        }

        widget_t *hovered = win->get_widget_at_pos(m_mouse_pos);
        
        if (hovered) {
            hovered->is_hovered = true;
        }
    }
    
}

// 
void window_manager_t::on_ui_window_close_event(const event_t &_e)
{
    window_handle_t handle = _e.as.ui_window_close.handle;
    release_window(handle);
    
}

//
window_handle_t window_manager_t::add_window(window_t &_window)
{
    // search for duplicates
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        if (m_pool[i].m_is_active && _window.name == m_pool[i].name) {
            return { i + 1 };
        }
    }

    // find first free slot
    uint32_t handle_slot = 0;
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        if (!m_pool[i].m_is_active) {
            // handle { 0 } is invalid, so adding 1
            handle_slot = i + 1;
            break;
        }
    }
    
    // check overflow
    if (handle_slot == 0) {
        SYN_WARNING("SYN_MAX_WINDOW_COuNT reached. New window creation rejected.\n");
        return { 0 };
    }
    
    m_pool[handle_slot - 1] = _window;
    window_handle_t handle = { handle_slot };
    
    window_t *win = &m_pool[handle_slot - 1];
    win->this_handle = handle;
    
    if (win->depth == 0.0f) {
        win->depth = m_next_depth;
        m_next_depth += m_ddepth_per_layer;
    } else if (win->depth >= m_next_depth) {
        m_next_depth = win->depth + m_ddepth_per_layer;
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
void window_manager_t::release_window(window_handle_t _handle)
{
    uint32_t idx = _handle.id - 1;
    if (_handle.id > 0 && _handle.id < SYN_MAX_WINDOW_COUNT) {
        window_t *win = &m_pool[idx];
        if (win->m_is_active) {
            win->m_is_active = false;
            win->destroy();
            m_pool[idx] = window_t();
        }
    }    
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
void window_manager_t::set_viewport_window(const window_handle_t &_handle)
{
    window_t *win = get_window(_handle);
    if (!win) {
        SYN_ERROR("invalid window handle for viewport.\n");
        m_viewport_window_handle = { 0 };
        return;
    }

    if (m_viewport_window_handle.id != 0 && m_viewport_window_handle.id != _handle.id) {
        SYN_INFO("replacing viewport.\n");
    }

    m_viewport_window_handle = _handle;

    SYN_INFO("set viewport window to '%s'.\n", win->name.c_str());
    
}

// 
window_t *window_manager_t::get_viewport_window()
{
    if (m_viewport_window_handle.id == 0) return nullptr;

    window_t *win = get_window(m_viewport_window_handle);

    if (!win || !win->is_active() || !win->is_visible()) {
        m_viewport_window_handle = { 0 };
        return nullptr;
    }

    return win;
}

//
window_handle_t window_manager_t::get_window_at_pos(const glm::vec2 _pos)
{
    // serach from highest depth
    window_handle_t top_window = { 0 };
    float highest_depth = m_zfar;
    
    for (uint32_t i = 0; i < m_active_count; i++) {
        window_t *win = &m_pool[i];
        if (!win->is_active() || !win->is_visible()) continue;
    
        if (win->is_point_in_window(_pos)) {
            if (win->depth >= highest_depth) {
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
    if (m_focused_window.id > 0) {
        window_t *prev = &m_pool[m_focused_window.id - 1];
        if (prev) {
            prev->set_focused(false);
            // prev->destroy();
            // prev->init();
        }
    }

    //  focus new window and bring to front
    if (_handle.id > 0) {
        window_t *win = &m_pool[_handle.id - 1];
        if (win) {
            win->set_focused(true);
            win->depth = m_next_depth;
            m_next_depth += 0.05;
        
            if (m_next_depth > 99.5f) {
                reorganize_depths();
            }
        }
    }
    
    m_focused_window = _handle;
}

//
void update_dock_zones(const glm::vec2 &_mouse_pos, const window_handle_t &_dragged_window_handle)
{
    
}

//
void window_manager_t::draw_windows()
{
    api.set_depth_testing(true);
    api.set_depth_func(GL_LEQUAL);
    api.set_depth_mask(GL_TRUE);

    m_projection = glm::ortho(0.0f, root_window.get_fwidth(), 
                              root_window.get_fheight(), 0.0f, 
                              m_zfar, m_znear);
    
    // 1. draw all colored geometry
    ui_batch_renderer.begin_batch();

    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (win->is_active() && win->is_visible()) {
            win->draw();
        }
    }

    ui_batch_renderer.end_batch();

    // 2. draw all textured content.
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (win->is_active() && win->is_visible() && win->has_frambuffer()) {
            draw_framebuffer(win);
        }
    }
    
    // 3. draw text
    font.end_render_block(true);
    
}

// 
void window_manager_t::draw_framebuffer(window_t *_win)
{
    printf("Content pos: (%.1f, %.1f), size: (%.1f, %.1f), depth: %.3f\n", 
        _win->get_content_position().x, _win->get_content_position().y,
        _win->get_content_size().x, _win->get_content_size().y,
        _win->depth + m_ddepth_layer_texture);

    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(_win->m_framebuffer_handle);
    if (!fbo) return;
    printf("FBO size: (%d, %d), Window content size: (%.0f, %.0f)\n",
        fbo->get_width(), fbo->get_height(),
        _win->get_content_size().x, _win->get_content_size().y);
    
    shader_t *shader = shader_lib.get_shader(m_tex_quad_shader_handle);
    if (!shader) return;
    
    shader->enable();
    shader->set_matrix_4fv("u_projection", m_projection);
    shader->set_uniform_2fv("u_position", _win->get_content_position());
    shader->set_uniform_2fv("u_size", _win->get_content_size());
    shader->set_uniform_1f("u_depth", _win->depth + m_ddepth_layer_texture);

    fbo->bind_texture(0, 0);

    m_tex_quad_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_tex_quad_vao.unbind();
        
    shader->disable();
    
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
    
    float new_depth = m_zfar + 1.0f + m_ddepth_per_layer * m_active_count;
    float closest_depth = new_depth;
    for (auto &pair : active_windows) {
        window_t *win = &m_pool[pair.index];
    
        // only update if depth change
        if (std::abs(win->depth - new_depth) > 0.01f) {
            win->depth = new_depth;
        }
        // TODO : invert to -=???
        new_depth += m_ddepth_per_layer;
    }
    
    // m_next_depth = closest_depth - m_ddepth_per_layer;
    m_next_depth = closest_depth + m_ddepth_per_layer;
}


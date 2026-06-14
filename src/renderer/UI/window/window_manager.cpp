
#include <vector>
#include <algorithm>

#include "renderer/UI/window/window_manager.h"
#include "renderer/entity/entity_types.h"
#include "utils/log.h"

#include "c_api.h"

//
static void __window_manager_on_mouse_button_callback(const event_t &_e) { window_manager.on_mouse_button_event(_e); }
static void __window_manager_on_mouse_move_callback(const event_t &_e) { window_manager.on_mouse_move_event(_e); }
static void __window_manager_on_mouse_scroll_callback(const event_t &_e) { window_manager.on_mouse_scroll_event(_e); }
static void __window_manager_on_keydown_callback(const event_t &_e) { window_manager.on_keydown_event(_e); }
static void __window_manager_on_input_char_callback(const event_t &_e) { window_manager.on_input_char_event(_e); }
static void __window_manager_on_ui_window_close_callback(const event_t &_e) { window_manager.on_ui_window_close_event(_e); }

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
    events.register_callback(event_type_t::INPUT_MOUSE_BUTTON, __window_manager_on_mouse_button_callback);
    events.register_callback(event_type_t::INPUT_MOUSE_MOVE, __window_manager_on_mouse_move_callback);
    events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, __window_manager_on_mouse_scroll_callback);
    events.register_callback(event_type_t::INPUT_KEYDOWN, __window_manager_on_keydown_callback);
    events.register_callback(event_type_t::INPUT_CHAR, __window_manager_on_input_char_callback);
    events.register_callback(event_type_t::UI_WINDOW_CLOSE, __window_manager_on_ui_window_close_callback);
    
    if (!renderer_2d.batch.is_initalized()) {
        renderer_2d.batch.init();
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
                set_focused_window(clicked);
                if (clicked_widget->type == widget_type_t::HIERARCHY) {
                    hierarchy_widget_t &hw = clicked_widget->hierarchy_widget;
                    float row_h = hw.row_height > 0.0f ? hw.row_height : font.get_font_glyph_height() + 6.0f;
                    glm::vec2 wp = clicked_widget->get_absolute_position(
                        glm::vec2(win->position.x, win->position.y + win->title_bar_height), 
                        glm::vec2(win->size.x, win->size.y - win->title_bar_height));
                    int row = (int)((pos.y - wp.y) / row_h) + (int)hw.scroll_offset;

                    uint32_t found = 0;
                    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
                        entity_t *e = &entity_lib.m_pool[i];
                        if (!e->is_active) continue;
                        if ((int)found == row) {
                            if (hw.selected) *hw.selected = { i + 1 };
                            break;
                        }
                        found++;
                    }
                    if (clicked_widget->consumes_click) return;
                }
                
                if (clicked_widget->type == widget_type_t::FLOAT_FIELD) {
                    // start scrub (in case), edit or scrub mode decided on release
                    m_is_scrubbing = true;
                    m_scrub_widget = clicked_widget;
                    m_scrub_start_pos = pos;
                    m_scrub_start_val = clicked_widget->float_field.value;
                    if (clicked_widget->consumes_click) return;
                }
                else {
                    if (clicked_widget->on_click) {
                        clicked_widget->on_click(clicked_widget);
                    }
                    if (clicked_widget->consumes_click) return;
                }
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

            // tab selection
            if (win->m_is_tab_container) {
                int tab_idx = win->get_tab_index_at_pos(pos);
                if (tab_idx >= 0) {
                    win->m_active_tab = (uint32_t)tab_idx;
                }
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
            if (clicked.id != m_focused_window_handle.id) {
                set_focused_window(clicked);
                return;
            }
        } else {
            set_focused_window({ 0 });
        }
    
    } else if (action == SYN_MOUSE_BUTTON_RELEASED) {
        if (m_is_scrubbing) {
            float dist = glm::abs(pos.x - m_scrub_start_pos.x);
            if (dist < 3.0f) {
                float_field_t &ff = m_scrub_widget->float_field;
                ff.editing = true;
                snprintf(ff.buf, sizeof(ff.buf), "%.3f", ff.value);
                ff.cursor = (int)strlen(ff.buf);
            }
            // if dragged, value is already updated; just commit
            m_is_scrubbing = false;
            m_scrub_widget = nullptr;
        }

        if (m_is_dragging) {

            if (m_enable_docking && m_hovered_dock_zone != dock_zone_t::NONE) {
                apply_docking(m_drag_window_handle, m_hovered_dock_zone, m_dock_target_window);
            }
            
            m_is_dragging = false;
            m_drag_window_handle = { 0 };
            m_show_dock_zones = false;
            m_hovered_dock_zone = dock_zone_t::NONE;
            m_dock_target_window = { 0 };
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
    m_hovered_window_handle = get_window_at_pos(m_mouse_pos);
    if (m_hovered_window_handle.id > 0) {
        window_t *win = &m_pool[m_hovered_window_handle.id - 1];
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
            win->on_resize();
            // 
            // win->resize_framebuffer();
            
            return;
        }
    }

    // adjusting float field value
    if (m_is_scrubbing && m_scrub_widget) {
        float_field_t &ff = m_scrub_widget->float_field;
        float delta = m_mouse_pos.x - m_scrub_start_pos.x;
        float speed = (input.is_key_down(SYN_KEY_LEFT_SHIFT)) ? 0.001f : (input.is_key_down(SYN_KEY_LEFT_CTRL)) ? 0.1f : 0.01f;
        float new_val = glm::clamp(m_scrub_start_val + delta * speed, ff.min, ff.max);
        ff.value = new_val;
        if (ff.binding) *ff.binding = new_val;
        if (ff.on_change) ff.on_change(new_val);
        return;
    }
    
    // moving windows -- the ui_batch_renderer takes care of the rendering
    if (m_is_dragging && m_drag_window_handle.id > 0) {
        update_dock_zones(m_mouse_pos, m_drag_window_handle);

        window_t *win = &m_pool[m_drag_window_handle.id - 1];
        win->position = m_mouse_pos - m_drag_offset;

        return;
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
void window_manager_t::on_mouse_scroll_event(const event_t &_e)
{
    float delta = _e.as.mouse_scroll.yoffset;

    window_handle_t hovered = get_window_at_pos(m_mouse_pos);
    if (hovered.id == 0) return;

    window_t *win = get_window(hovered);
    if (!win) return;

    widget_t *w = win->get_widget_at_pos(m_mouse_pos);
    if (w && w->on_scroll) {
        w->on_scroll(w, delta);
        return;
    }
}

// 
void window_manager_t::on_keydown_event(const event_t &_e)
{
    int action = _e.as.keydown.action;
    if (action != SYN_KEY_PRESSED && action != SYN_KEY_REPEAT) return;
    if (_e.as.keydown.focused_window_handle.id == 0) return;

    // route keys into focused window
    window_t *win = get_window(_e.as.keydown.focused_window_handle);
    int key = _e.as.keydown.key;
    int mods = _e.as.keydown.mods;

    if (key == SYN_KEY_TAB) {
        if (win->m_is_tab_container) {
            bool shift = (mods & SYN_MOD_SHIFT);
            bool ctrl = (mods & SYN_MOD_CTRL);
            if (!ctrl) return;
            if (shift) {
                // trevious tab
                win->m_active_tab = (win->m_active_tab == 0) ? win->m_tab_count - 1 : win->m_active_tab - 1;
            } else {
                // next tab
                win->m_active_tab = (win->m_active_tab + 1) % win->m_tab_count;
            }
        }
    }

    // float field keyboard handling
    for (uint32_t i = 0; i < win->m_widget_count; i++) {
        widget_t *w = &win->m_widgets[i];
        if (w->type != widget_type_t::FLOAT_FIELD || !w->float_field.editing) continue;

        float_field_t &ff = w->float_field;
        int len = (int)strlen(ff.buf);

        switch (key)
        {
            case SYN_KEY_ENTER: {
                char *end;
                float v = strtof(ff.buf, &end);
                if (end != ff.buf) {
                    v = glm::clamp(v, ff.min, ff.max);
                    ff.value = v;
                    if (ff.binding) *ff.binding = v;
                    if (ff.on_change) ff.on_change(v);
                }
                ff.editing = false;
                break;
            }
            
            case SYN_KEY_ESCAPE: {
                ff.editing = false;
                break;
            }

            case SYN_KEY_BACKSPACE: {
                if (ff.cursor > 0) {
                    memmove(&ff.buf[ff.cursor - 1], &ff.buf[ff.cursor], len - ff.cursor + 1);
                    ff.cursor--;
                }
                break;
            }

            case SYN_KEY_DELETE: {
                if (ff.cursor < len) {
                    memmove(&ff.buf[ff.cursor], &ff.buf[ff.cursor + 1], len - ff.cursor);
                }
                break;
            }

            case SYN_KEY_LEFT: {
                if (ff.cursor > 0) ff.cursor--;
                break;
            }

            case SYN_KEY_RIGHT: {
                if (ff.cursor < len) ff.cursor++;
                break;
            }

            default: break;
            
        }

        // only one float field can be in edit mode, yes?
        return;
    }
    
}

// 
void window_manager_t::on_input_char_event(const event_t &_e)
{
    if (_e.as.input_char.focused_window_handle.id == 0) return;
    window_t *win = get_window(_e.as.input_char.focused_window_handle);
    if (!win) return;

    unsigned int cp = _e.as.input_char.codepoint;
    if (!((cp >= '0' && cp <= '9') || cp == '.' || cp == '-')) return;

    for (uint32_t i = 0; i < win->m_widget_count; i++) {
        widget_t *w = &win->m_widgets[i];
        if (w->type != widget_type_t::FLOAT_FIELD || !w->float_field.editing) continue;

        float_field_t &ff = w->float_field;
        int len = (int)strlen(ff.buf);
        if (len >= 31) return;

        memmove(&ff.buf[ff.cursor + 1], &ff.buf[ff.cursor], len - ff.cursor + 1);
        ff.buf[ff.cursor] = (char)cp;
        ff.cursor++;

        return;
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

        if (!win->m_is_active) return;

        // case 1: window is a tab container -- release children first
        if (win->m_is_tab_container) {
            window_handle_t children[SYN_WINDOW_MAX_TABS];
            uint32_t count = win->m_tab_count;
            for (uint32_t i = 0; i < count; i++) {
                children[i] = win->m_tab_children[i];
            }

            for (uint32_t i = 0; i < count; i++) {
                if (children[i].id != _handle.id) {
                    release_window(children[i]);
                }
            }
        }

        // case 2: releaseing a tab child, remove from parent
        if (win->m_is_tab_child && win->m_tab_parent.id != 0) {
            window_t *parent = get_window(win->m_tab_parent);
            if (parent && parent->m_is_tab_container) {
                // find and remove from parents child array
                for (uint32_t i = 0; i < parent->m_tab_count; i++) {
                    if (parent->m_tab_children[i].id == _handle.id) {
                        // shift down remaining
                        for (uint32_t j = i; j < parent->m_tab_count; j++) {
                            parent->m_tab_children[j] = parent->m_tab_children[j + 1];
                        }
                        parent->m_tab_children[--parent->m_tab_count] = { 0 };
                        break;
                    }
                }

                // clamp active tab in case we removed the last one or the active one
                if (parent->m_active_tab >= parent->m_tab_count && parent->m_tab_count > 0) {
                    parent->m_active_tab = parent->m_tab_count - 1;
                }

                // de-convert parent if only one tab remains
                if (parent->m_tab_count == 1) {
                    window_handle_t remaining_handle = parent->m_tab_children[0];
                    window_t *remaining = get_window(remaining_handle);
                    if (remaining) {
                        // promote remaining child back to standalone window
                        remaining->m_is_tab_child = false;
                        remaining->m_is_visible = true;
                        remaining->m_tab_parent = { 0 };
                        remaining->title_bar_height = remaining->m_original_title_bar_height;
                        remaining->position = parent->position;
                        remaining->size = parent->size;
                        remaining->depth = parent->depth;
                        remaining->on_resize();
                    }
                    // de-convert the container
                    parent->m_is_tab_container = false;
                    parent->m_tab_count = 0;
                    parent->m_tab_children[0] = { 0 };
                    parent->m_active_tab = 0;
                }

                // if no tabs left, release the container itself
                if (parent->m_tab_count == 0) {
                    release_window(win->m_tab_parent);
                    win->m_tab_parent = { 0 };
                }
            }
        }

        // clear viewport handle if this window was the viewport
        if (m_viewport_window_handle.id == _handle.id) {
            m_viewport_window_handle = { 0 };
        }

        // clear focus/drag/hover ids
        if (m_focused_window_handle.id == _handle.id)   m_focused_window_handle = { 0 };
        if (m_hovered_window_handle.id == _handle.id)   m_hovered_window_handle = { 0 };
        if (m_drag_window_handle.id == _handle.id)      m_drag_window_handle    = { 0 };
        if (m_resize_window_handle.id == _handle.id)    m_resize_window_handle  = { 0 };
        
        // 
        win->m_is_active = false;
        win->destroy();
        m_pool[idx] = window_t();
        m_active_count--;
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

    // 
    api.set_scene_viewport(win->get_content_size());
    
    
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

    if (win->m_is_tab_container) {
        return get_active_tab_child(win);
    }
    
    return win;
}

//
window_handle_t window_manager_t::get_window_at_pos(const glm::vec2 _pos)
{
    // serach from highest depth
    window_handle_t top_window = { 0 };
    float highest_depth = m_zfar;
    
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
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
window_handle_t window_manager_t::get_window_at_pos(const glm::vec2 _pos, 
                                                    const window_handle_t &_exclude_handle)
{
    window_handle_t top_window = { 0 };
    float highest_depth = m_zfar;

    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (!win->is_active() || !win->is_visible()) continue;
        if (win->this_handle.id == _exclude_handle.id) continue;

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
    if (m_focused_window_handle.id > 0) {
        window_t *prev = &m_pool[m_focused_window_handle.id - 1];
        if (prev) {
            prev->set_focused(false);
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
    
    m_focused_window_handle = _handle;
}

//
void window_manager_t::update_dock_zones(const glm::vec2 &_mouse_pos, const window_handle_t &_dragged_window_handle)
{
    m_show_dock_zones = true;
    m_hovered_dock_zone = dock_zone_t::NONE;
    m_dock_target_window = { 0 };

    float screen_w = root_window.get_fwidth();
    float screen_h = root_window.get_fheight();
    float margin = m_dock_zone_margin;
    glm::vec2 mpos = _mouse_pos;

    // clear all zones
    for (uint32_t i = 0; i < 5; i++) {
        m_dock_zones[i].is_hovered = false;
    }

    m_dock_zones[0].zone = dock_zone_t::LEFT;
    m_dock_zones[0].bounds = glm::vec4(0, 0, margin * 2, screen_h);
    
    m_dock_zones[1].zone = dock_zone_t::RIGHT;
    m_dock_zones[1].bounds = glm::vec4(screen_w - margin * 2, 0, margin * 2, screen_h);
    
    m_dock_zones[2].zone = dock_zone_t::TOP;
    m_dock_zones[2].bounds = glm::vec4(0, 0, screen_w, margin * 2);

    m_dock_zones[3].zone = dock_zone_t::BOTTOM;
    m_dock_zones[3].bounds = glm::vec4(0, screen_h - margin * 2, screen_w, margin * 2);

    // check screen margins
    for (uint32_t i = 0; i < 4; i++) {
        glm::vec4 b = m_dock_zones[i].bounds;
        if (mpos.x >= b.x && mpos.x <= b.x + b.z &&
            mpos.y >= b.y && mpos.y <= b.y + b.w) {
            m_dock_zones[i].is_hovered = true;
            m_hovered_dock_zone = m_dock_zones[i].zone;
            return;
        }
    }

    // check center zone
    window_handle_t hovered = get_window_at_pos(mpos, _dragged_window_handle);
    if (hovered.id != 0 && hovered.id != _dragged_window_handle.id) {
        window_t *target = get_window(hovered);
        if (target) {
            m_dock_zones[4].zone = dock_zone_t::CENTER;
            m_dock_zones[4].bounds = glm::vec4(
                target->position.x + margin,
                target->position.y + margin,
                target->size.x - margin * 2,
                target->size.y - margin * 2
            );

            glm::vec4 b = m_dock_zones[4].bounds;
            if (mpos.x >= b.x && mpos.x <= b.x + b.z &&
                mpos.y >= b.y && mpos.y <= b.y + b.w) {
                m_dock_zones[4].is_hovered = true;
                m_hovered_dock_zone = dock_zone_t::CENTER;
                m_dock_target_window = hovered;
                return;
            }
        }
    }
}

// 
void window_manager_t::apply_docking(window_handle_t _handle, dock_zone_t _zone, window_handle_t _target_handle)
{
    window_t *win = get_window(_handle);
    if (!win) return;

    float screen_w = root_window.get_fwidth();
    float screen_h = root_window.get_fheight();
    float padding = 0.0f;

    switch (_zone) {
        case dock_zone_t::LEFT:
            win->position = glm::vec2(padding, padding);
            win->size = glm::vec2(screen_w * 0.5f - padding * 1.5f, screen_h - padding * 2);
            break;
            
        case dock_zone_t::RIGHT:
            win->position = glm::vec2(screen_w * 0.5f + padding * 0.5f, padding);
            win->size = glm::vec2(screen_w * 0.5f - padding * 1.5f, screen_h - padding * 2);
            break;
            
        case dock_zone_t::TOP:
            win->position = glm::vec2(padding, padding);
            win->size = glm::vec2(screen_w - padding * 2, screen_h * 0.5f - padding * 1.5f);
            break;
            
        case dock_zone_t::BOTTOM:
            win->position = glm::vec2(padding, screen_h * 0.5f + padding * 0.5f);
            win->size = glm::vec2(screen_w - padding * 2, screen_h * 0.5f - padding * 1.5f);
            break;
            
        case dock_zone_t::CENTER:
            if (_target_handle.id != 0) {
                dock_as_tab(_handle, _target_handle);
            }
            return;
            break;
            
        default: break;
    }

    win->on_resize();

}

void window_manager_t::dock_as_tab(window_handle_t _new, window_handle_t _target)
{
    window_t *target_win = get_window(_target);
    window_t *new_win = get_window(_new);
    if (!target_win || !new_win) return;

    // convert to container if needed
    if (!target_win->m_is_tab_container) {
        target_win->m_is_tab_container = true;
        target_win->on_resize();
        target_win->m_tab_children[0] = _target;
        target_win->m_tab_count = 1;
        target_win->m_active_tab = 0;
    }

    if (target_win->m_tab_count < SYN_WINDOW_MAX_TABS) {
        new_win->m_is_tab_child = true;
        new_win->m_is_visible = false;
        new_win->m_tab_parent = _target;
        new_win->m_original_title_bar_height = new_win->title_bar_height;
        new_win->title_bar_height = 0.0f;
        target_win->m_tab_children[target_win->m_tab_count++] = _new;

        new_win->position = target_win->position;
        new_win->size = target_win->size;
        new_win->on_resize();
        // if (new_win->has_frambuffer()) {
        //     new_win->resize_framebuffer();
        // }
    }
    
}

// 
window_t *window_manager_t::get_active_tab_child(window_t *_window)
{
    window_handle_t child_handle = _window->get_active_tab_child_handle();
    return get_window(child_handle);
    
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

    // for now, we hard code syncing the property window at this level
    window_t *pw = get_window(m_properties_window_handle);
    if (pw && selected_entity_handle.is_valid()) {
        entity_t *e = entity_lib.get_entity(selected_entity_handle);
        if (e) {
            float vals[9] = {
                e->t_position.x, e->t_position.y, e->t_position.z,
                e->t_rotation.x, e->t_rotation.y, e->t_rotation.z,
                e->t_scale.x,    e->t_scale.y,    e->t_scale.z,
            };
            // widgets: 0=close_btn, 1=label, 2=hierarchy, 3=label, 4-12=float_field
            for (int i = 0; i < 9; i++) {
                widget_t *w = pw->get_widget(i + 4);
                if (w && w->type == widget_type_t::FLOAT_FIELD && !w->float_field.editing) {
                    w->float_field.value = vals[i];
                } 
            }
        }
    }
    
    // 1. draw all colored geometry
    renderer_2d.batch.begin_batch();

    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (win->is_active() && win->is_visible()) {
            win->draw();
        }
    }

    // 
    draw_dock_zone_overlays();

    // draw all quads / line_strips
    renderer_2d.batch.end_batch();

    // 2. draw all textured content.
    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = &m_pool[i];
        if (!win->is_active() || !win->is_visible() || win->m_is_tab_child) continue;

        if (win->m_is_tab_container) {
            window_t *active = get_active_tab_child(win);
            if (active && active->has_frambuffer()) {
                draw_framebuffer_for(active, win);
            }
        }
        else if (win->has_frambuffer()) {
            draw_framebuffer(win);
        }
    }
    
    // 3. draw text
    font.end_render_block(true);
    
}

// 
void window_manager_t::draw_framebuffer(window_t *_win)
{
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(_win->get_framebuffer_handle());
    if (!fbo) return;
 
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
void window_manager_t::draw_framebuffer_for(window_t *_active, window_t *_tab_container)
{
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(_active->get_framebuffer_handle());
    if (!fbo) return;
 
    shader_t *shader = shader_lib.get_shader(m_tex_quad_shader_handle);
    if (!shader) return;
    
    shader->enable();
    shader->set_matrix_4fv("u_projection", m_projection);
    shader->set_uniform_2fv("u_position", _tab_container->get_content_position());
    shader->set_uniform_2fv("u_size", _tab_container->get_content_size());
    shader->set_uniform_1f("u_depth", _tab_container->depth + m_ddepth_layer_texture);

    fbo->bind_texture(0, 0);

    m_tex_quad_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_tex_quad_vao.unbind();
        
    shader->disable();
    
}

// 
void window_manager_t::draw_dock_zone_overlays()
{
    if (!m_show_dock_zones) return;
    
    for (int i = 0; i < 5; i++) {
        if (!m_dock_zones[i].is_hovered) continue;
        
        glm::vec4 b = m_dock_zones[i].bounds;
        glm::vec4 color = glm::vec4(0.65f, 0.30f, 0.04f, m_dock_preview_alpha);
        
        //
        renderer_2d.batch.add_quad(glm::vec2(b.x, b.y), glm::vec2(b.z, b.w), color, m_znear - 0.5f);
        
        // Draw outline
        glm::vec4 outline_color = glm::vec4(1.0f, 0.55f, 0.05f, 0.8f);
        float x0 = b.x + 1.0f;
        float y0 = b.y + 1.0f;
        float x1 = b.x + b.z - 1.0f;
        float y1 = b.y + b.w - 1.0f;
        
        ui_render_vertex_t outline[] = {
            ui_render_vertex_t({ x0, y0 }, outline_color, m_znear - 0.1f),
            ui_render_vertex_t({ x1, y0 }, outline_color, m_znear - 0.1f),
            ui_render_vertex_t({ x1, y1 }, outline_color, m_znear - 0.1f),
            ui_render_vertex_t({ x0, y1 }, outline_color, m_znear - 0.1f),
            ui_render_vertex_t({ x0, y0 }, outline_color, m_znear - 0.1f),
        };

        renderer_2d.batch.add_line_strip(outline, 5);
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
    
    float new_depth = m_zfar + 1.0f + m_ddepth_per_layer * m_active_count;
    float closest_depth = new_depth;
    for (auto &pair : active_windows) {
        window_t *win = &m_pool[pair.index];
    
        // only update if depth change
        if (std::abs(win->depth - new_depth) > 0.01f) {
            win->depth = new_depth;
        }
        new_depth += m_ddepth_per_layer;
    }
    
    m_next_depth = closest_depth + m_ddepth_per_layer;
}


#ifndef __WINDOW_MANAGER_H
#define __WINDOW_MANAGER_H

#include "renderer/UI/window/window.h"
#include "renderer/shader/shader_types.h"
#include "event/event.h"

// 
#define SYN_MAX_WINDOW_COUNT    64

// 
class window_manager_t
{
public:
    friend class window_t;
    friend class ui_render_batch_t;
    
public:
    window_manager_t() = default;
    ~window_manager_t() = default;

    void init();
    void shutdown();

    void on_mouse_button_event(const event_t &_e);
    void on_mouse_move_event(const event_t &_e);

    window_handle_t add_window(window_t &_window);
    window_handle_t add_window(const window_desc_t &_desc);
    
    window_t *get_window(const window_handle_t &_handle);

    // interaction
    window_handle_t get_window_at_pos(const glm::vec2 _pos);
    void set_focused_window(window_handle_t _handle);
    const window_handle_t &get_focused_window() { return m_focused_window; }

    void draw_windows();

// private:
    void reorganize_depths();
    // void init_drag_vao();
    // void update_drag_vao(const glm::vec2 &_pos);
    
private:
    window_t m_pool[SYN_MAX_WINDOW_COUNT];
    size_t m_active_count = 0;

    shader_handle_t m_window_shader_handle;

    glm::mat4 m_projection;
    
    // window focus and depth
    window_handle_t m_focused_window = { 0 };
    float m_zfar = -100.0f;
    float m_znear = 100.0f;
    // assigns 0.05f per layer, leaving room for text rendering
    float m_ddepth_per_layer = 0.05f;
    float m_ddepth_layer_text = 0.01f;
    float m_next_depth = m_zfar;

    // moving windows
    bool m_is_dragging = false;
    window_handle_t m_drag_window_handle = { 0 };
    glm::vec2 m_mouse_pos;
    glm::vec2 m_drag_offset;
    
};





#endif // __WINDOW_MANAGER_H

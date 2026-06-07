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
    
public:
    window_manager_t() = default;
    ~window_manager_t() = default;

    void init();
    void shutdown();

    window_handle_t add_window(window_t &_window);
    window_handle_t add_window(const window_desc_t &_desc);
    
    window_t *get_window(const window_handle_t &_handle);

    // interaction
    window_handle_t get_window_at_pos(float _x, float _y);
    void set_focused_window(window_handle_t _handle);
    void on_mouse_button_event(const event_t &_e);
    const window_handle_t &get_focused_window() { return m_focused_window; }

    void draw_windows();

// private:
    void reorganize_depths();
    
private:
    window_t m_pool[SYN_MAX_WINDOW_COUNT];
    size_t m_active_count = 0;

    shader_handle_t m_window_shader_handle;

    // 
    window_handle_t m_focused_window = { 0 };
    float m_zfar = 100.0f;
    float m_znear = -100.0f;
    // assigns 0.05f per layer, leaving room for title (-0.02f) and font (-0.03f)
    float m_ddepth_per_layer = 0.05f;
    float m_next_depth = m_zfar;
    
};





#endif // __WINDOW_MANAGER_H

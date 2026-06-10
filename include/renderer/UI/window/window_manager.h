#ifndef __WINDOW_MANAGER_H
#define __WINDOW_MANAGER_H

#include "renderer/UI/window/window.h"
#include "renderer/shader/shader_types.h"
#include "renderer/buffers/vertex_array.h"
#include "event/event.h"

// 
#define SYN_MAX_WINDOW_COUNT    32

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
    void on_keydown_event(const event_t &_e);
    void on_ui_window_close_event(const event_t &_e);

    // window creation/access
    window_handle_t add_window(window_t &_window);
    window_handle_t add_window(const window_desc_t &_desc);
    void release_window(window_handle_t _handle);    
    window_t *get_window(const window_handle_t &_handle);

    // viewports
    void set_viewport_window(const window_handle_t &_handle);
    window_t *get_viewport_window();
    window_handle_t get_viewport_window_handle() { return m_viewport_window_handle; }
    bool has_viewport_window() { return m_viewport_window_handle.id != 0; }

    // interaction
    window_handle_t get_window_at_pos(const glm::vec2 _pos);
    window_handle_t get_window_at_pos(const glm::vec2 _pos, const window_handle_t &_exclude_handle);
    void set_focused_window(window_handle_t _handle);
    const window_handle_t &get_focused_window() { return m_focused_window_handle; }

    // docking
    void update_dock_zones(const glm::vec2 &_mouse_pos, const window_handle_t &_dragged_window_handle);
    void apply_docking(window_handle_t _handle, dock_zone_t _zone, window_handle_t _target_handle);
    void dock_as_tab(window_handle_t _new, window_handle_t _target);

    // tabs
    window_t *get_active_tab_child(window_t *_window);
    void add_tab(const window_handle_t &_container_handle, const window_handle_t &_child_handle);
    void set_active_tab(const window_handle_t &_container_handle, uint32_t _index);
    
    
    // drawing
    void draw_windows();
    void draw_framebuffer(window_t *_window);
    void draw_framebuffer_for(window_t *_active, window_t *_tab_container);
    void draw_dock_zone_overlays();
    
// private:
    void reorganize_depths();
    
private:
    window_t m_pool[SYN_MAX_WINDOW_COUNT];
    size_t m_active_count = 0;

    shader_handle_t m_window_shader_handle;

    window_handle_t m_viewport_window_handle = { 0 };
    shader_handle_t m_tex_quad_shader_handle;
    vertex_array_t m_tex_quad_vao;
    
    glm::mat4 m_projection;
    
    // window focus and depth
    window_handle_t m_hovered_window_handle = { 0 };
    window_handle_t m_focused_window_handle = { 0 };
    float m_zfar = -100.0f;
    float m_znear = 100.0f;
    
    float m_ddepth_per_layer = 0.05f;       // assign 0.05f per layer, leaving room for text and frambuffer rendering
    float m_ddepth_layer_text = 0.02f;
    float m_ddepth_layer_texture = 0.01f;
    float m_next_depth = m_zfar;

    // moving windows
    bool m_is_dragging = false;
    window_handle_t m_drag_window_handle = { 0 };
    glm::vec2 m_mouse_pos;
    glm::vec2 m_drag_offset;

    // resizing windows
    bool m_is_resizing = false;
    window_handle_t m_resize_window_handle = { 0 };
    resize_handle_t m_resize_handle = resize_handle_t::NONE;
    glm::vec2 m_resize_start_pos = glm::vec2(0.0f);
    glm::vec2 m_resize_start_size = glm::vec2(0.0f);
    glm::vec2 m_resize_start_window_pos = glm::vec2(0.0f);

    // docking
    bool m_enable_docking = true;
    float m_dock_preview_alpha = 0.3f;
    float m_dock_zone_margin = 50.0f;
    bool m_show_dock_zones = false;
    dock_zone_visual_t m_dock_zones[5];
    dock_zone_t m_hovered_dock_zone = dock_zone_t::NONE;
    window_handle_t m_dock_target_window = { 0 };
    
};





#endif // __WINDOW_MANAGER_H

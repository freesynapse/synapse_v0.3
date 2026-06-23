#ifndef __WINDOW_H
#define __WINDOW_H

#include <vector>

#include "renderer/UI/window/window_types.h"
#include "renderer/buffers/framebuffer_types.h"
#include "renderer/UI/window/widget_state.h"


// 
#define SYN_WINDOW_MAX_TABS 8

// 
class window_t
{
public:
    friend class window_manager_t;
    friend class editor_t;
    
public:
    window_t() = default;
    ~window_t() = default;

    void init();
    void destroy();

    void create_framebuffer();
    void resize_framebuffer();
    void destroy_framebuffer();

    void on_resize();
    
    void draw();
    void draw_tab_bar();
    
    // interaction
    bool is_point_in_window(const glm::vec2 &_p);
    bool is_point_in_title_bar(const glm::vec2 &_p);

    resize_handle_t get_resize_handle_at_pos(const glm::vec2 &_pos);
    
    // accessors
    const window_handle_t &handle() { return this_handle; }

    const bool &is_active() { return m_is_active; }
    const bool &is_visible() { return m_is_visible; }
    const bool &is_focused() { return m_is_focused; }
    const bool &is_movable() { return m_is_movable; }
    const bool &is_resizable() { return m_is_resizable; }

    void set_active(bool _b) { m_is_active = _b; }
    void set_visible(bool _b) { m_is_visible = _b; }
    void set_focused(bool _b) { m_is_focused = _b; }
    void set_movable(bool _b) { m_is_movable = _b; }
    void set_resizable(bool _b) { m_is_resizable = _b; }
    
    const bool &has_frambuffer() { return m_has_framebuffer; }
    const framebuffer_handle_t &get_framebuffer_handle() { return m_framebuffer_handle; }

    void hide() { m_is_visible = false; }
    void show() { m_is_visible = true; }
    glm::vec2 get_content_size() { return glm::vec2(size.x, size.y - title_bar_height); }
    glm::vec2 get_content_position() { return glm::vec2(position.x, position.y + title_bar_height); }

    // immediate mode
    void im_begin(float _padding=4.0f);
    widget_state_t *im_get_or_create_state(uint32_t _id);
    bool im_is_hovered(const glm::vec2 &_pos, const glm::vec2 &_size);
    // rows
    void im_begin_row(const std::vector<float> &_ratios);
    void im_end_row();    
    // for clearing states at reload etc
    void im_clear_states();
    
    // tabs
    window_handle_t get_active_tab_child_handle();
    int get_tab_index_at_pos(const glm::vec2 &_pos);
    bool is_tab_container() { return m_is_tab_container; }
    bool is_tab_child() { return m_is_tab_child; }
    
public:
    // for events affecting this window dispatched from this window
    window_handle_t this_handle;
    
    // window params
    std::string name   = "";
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 size     = glm::vec2(0.0f);     // vec2(width, height)
    float depth        = 0.0f; // z depth [-100.0 ... 100.0]

    glm::vec4 bg_color      = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glm::vec4 fg_color      = glm::vec4(1.0f);
    glm::vec4 outline_color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);

    float title_bar_height            = 26.0f;
    glm::vec4 title_bar_color         = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
    glm::vec4 title_bar_color_focused = glm::vec4(0.65f, 0.30f, 0.04f, 1.0f);

    bool fit_to_content_height = false;
    bool fit_to_content_width  = false;
    bool is_scrollable         = false;
    
    // immediate mode widget state cache
    widget_state_t im_widget_states[SYN_IM_MAX_WIDGET_STATES];
    uint32_t       im_widget_state_count = 0;

    // immediate mode cursor -- tracks Y position during syn_begin/end_window
    float im_cursor_x           = 0.0f;
    float im_cursor_y           = 0.0f;
    float im_default_row_height = 24.0f;
    float im_padding            = 4.0f;

    // immediate mode input handling
    uint32_t  im_active_state_id = 0;
    bool      im_click_pending   = false;
    glm::vec2 im_click_pos       = glm::vec2(0.0f);
    float     im_scroll_delta    = 0.0f;

    // immediate mode row layout state
    bool               im_in_row       = false;
    int                im_col_index    = 0;
    float              im_row_saved_y  = 0.0f;
    float              im_row_max_h    = 0.0f;
    std::vector<float> im_col_x;
    std::vector<float> im_col_w;    

    // im dragging
    uint32_t im_dragging_state_id = 0;
    im_drag_type_t im_drag_type   = im_drag_type_t::NONE;
    glm::vec2 im_drag_widget_pos  = glm::vec2(0.0f);
    glm::vec2 im_drag_widget_size = glm::vec2(0.0f);
    
private:
    // constants
    float m_original_title_bar_height           = title_bar_height;
    
    // resizing
    float m_resize_border_width                 = 5.0f;
    glm::vec2 min_size                          = glm::vec2(100.0f, 100.0f);
    glm::vec2 max_size                          = glm::vec2(2560.0f, 1600.0f);

    // title bar buttons
    glm::vec2 m_close_btn_pos                   = glm::vec2(0.0f);
    glm::vec2 m_close_btn_size                  = glm::vec2(0.0f);
    
    // framebuffer members 
    bool m_has_framebuffer                      = false;
    framebuffer_handle_t m_framebuffer_handle   = { 0 };

    // tabs
    
    /*  Tab children are initiated with 
     *      m_is_visible = false
     *      m_is_tab_child = true
     *
     *  The originator window has
     *      m_is_tab_container = true
     *      m_is_visible = true
     *      m_tab_children index = 0
     *  and thus responsible for drawing the active tab
     */
    window_handle_t m_tab_children[SYN_WINDOW_MAX_TABS];    
    uint32_t m_tab_count                        = 0;
    uint32_t m_active_tab                       = 0;
    window_handle_t m_tab_parent;

    bool m_is_tab_container                     = false;
    bool m_is_tab_child                         = false;

    // general flags
    bool m_is_active                            = false;
    bool m_is_visible                           = false;
    bool m_is_focused                           = false;
    bool m_is_movable                           = true;
    bool m_is_resizable                         = true;
    
};


//---------------------------------------------------------------------------------------
// immediate mode helper functions 
// 
void im_set_widget_params(window_t *_window, widget_params_t *_params);
void im_update_cursor_y(window_t *_window, widget_params_t *_params);
bool im_needs_update(window_t *_window);
uint32_t im_widget_hash(const char *_str, window_t *_window);


#endif // __WINDOW_H

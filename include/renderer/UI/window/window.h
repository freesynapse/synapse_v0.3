#ifndef __WINDOW_H
#define __WINDOW_H

#include "renderer/UI/window/window_types.h"
#include "renderer/buffers/vertex_array.h"
#include "renderer/buffers/framebuffer.h"
#include "renderer/UI/window/widget_types.h"

// 

// The window class, rendered onto the renderer.m_scene_fbuffer
class window_t
{
public:
    friend class window_manager_t;
    
public:
    window_t() = default;
    ~window_t() = default;

    void init();
    void destroy();

    void draw();

    // interaction
    bool is_point_in_window(const glm::vec2 &_p);
    bool is_point_in_title_bar(const glm::vec2 &_p);
    
    // accessors
    const bool &is_active() { return m_is_active; }
    const bool &is_visible() { return m_is_visible; }
    const bool &is_focused() { return m_is_focused; }
    void set_focused(bool _focused) { m_is_focused = _focused; }
    const bool &has_frambuffer() { return m_has_framebuffer; }

    // widgets
    void add_widget(const widget_t &_widget);
    widget_t *get_widget(uint32_t _index);
    widget_t *get_widget_at_pos(const glm::vec2 &_pos);
    void draw_widgets();
    
public:
    // window params
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(0.0f);     // vec2(width, height)

    float depth = 0.0f; // z depth [-1..1]

    glm::vec4 bg_color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glm::vec4 fg_color = glm::vec4(1.0f);
    glm::vec4 outline_color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);

    float title_bar_height = 26.0f;
    glm::vec4 title_bar_color = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
    glm::vec4 title_bar_color_focused = glm::vec4(0.65f, 0.30f, 0.04f, 1.0f);

    bool fit_to_content_height = false;
    bool fit_to_content_width  = false;
    bool is_scrollable = false;
    
    std::string name = "";

private:
    widget_t m_widgets[SYN_WINDOW_MAX_WIDGET_COUNT];
    uint32_t m_widget_count = 0;

    vertex_array_t m_vao_body;
    vertex_array_t m_vao_outline;

    bool m_has_framebuffer = false;
    framebuffer_handle_t m_fbuffer = { 0 };

    bool m_is_active = false;
    bool m_is_visible = false;
    bool m_is_focused = false;
    
};



#endif // __WINDOW_H

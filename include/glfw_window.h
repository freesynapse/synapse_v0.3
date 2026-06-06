#ifndef __WINDOW_H
#define __WINDOW_H

#ifndef GLAD_INCLUDED
#include "gl_api.h"
#endif

#include "event/event.h"
#include "event/key_codes.h"


// TODO : remove as class? (not for now at least, see below)
struct glfw_window_t
{
    glfw_window_t() = default;
    ~glfw_window_t() = default;

    /*  If _width or _height is set to 0, default dimensions 1280 x 800 is used. */
    int init(const char *_name, int _width, int _height);
    void destroy();

    void pre_render();
    void post_render();
    void update();
    void center_cursor();

    // glfw callback
    void glfw_window_resize_callback(GLFWwindow* _window, int _width, int _height);

    // events callbacks
    void on_window_close_event(const event_t &_e);
    void on_toggle_fullscreen_event(const event_t &_e);
    void on_toggle_cursor_event(const event_t &_e);
    void on_toggle_frozen_cursor_event(const event_t &_e);
    void on_keydown_event(const event_t &_e);

    // accessors
    bool should_close() const { return (glfwWindowShouldClose(m_window_ptr) || m_to_close_window); }
    void set_exit_key(unsigned int _key) { m_to_close_key = _key; }
    void set_vsync(bool _b) { m_is_vsync = _b; glfwSwapInterval((int)_b); }
    void set_cursor_pos(const glm::vec2& _pos) { glfwSetCursorPos(m_window_ptr, _pos.x, _pos.y); }
    void close() { m_to_close_window = true; }
    void set_window_position(int _x, int _y) { m_window_position = { _x, _y }; glfwSetWindowPos(m_window_ptr, _x, _y); glfwPollEvents(); }
    void set_window_position(const glm::ivec2 &_pos) { m_window_position = _pos; glfwSetWindowPos(m_window_ptr, _pos.x, _pos.y); glfwPollEvents(); }
    void set_window_dim(const glm::ivec2 &_dims) { m_window_dim = _dims; glfwSetWindowSize(m_window_ptr, _dims.x, _dims.y); glfwPollEvents(); }
    void set_window_dim(int _w, int _h) { m_window_dim = { _w, _h }; glfwSetWindowSize(m_window_ptr, _w, _h); glfwPollEvents(); }
    void set_fullscreen(const bool& _fullscreen);
    void toggle_fullscreen() { set_fullscreen(!m_is_fullscreen); }
    float get_width() { return (float)m_window_dim.x; }
    float get_height() { return (float)m_window_dim.y; }
    
    //int width = 0;
    //int height = 0;
    glm::ivec2 m_window_dim = { 0, 0 };

    //int m_screen_width = 0;
    //int m_screen_height = 0;
    glm::ivec2 m_screen_dim = { 0, 0 };

    //int m_render_width = 0;
    //int m_render_height = 0;
    glm::ivec2 m_render_dim = { 0, 0 };

    int m_window_pos_x = 0;
    int m_window_pos_y = 0;
    glm::ivec2 m_window_position = { 0, 0 };
    
    const char* m_title = "syn2";

    GLFWwindow* m_window_ptr = NULL;
    GLFWmonitor* m_primary_monitor = NULL;
    const GLFWvidmode* video_mode;

    uint32_t m_to_close_key = SYN_KEY_ESCAPE;
    
    // flags
    bool m_is_closed = false;
    bool m_to_close_window = false;
    bool m_is_fullscreen = false;

    bool m_is_cursor_frozen = false;
    bool m_is_cursor_visible = true;
    bool m_is_vsync = true;

};


#endif // __WINDOW_H

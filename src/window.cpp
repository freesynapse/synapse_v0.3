
#include "window.h"
#include "core.h"
#include "utils/log.h"
#include "event/event.h"
#include "event/input_manager.h"

#include "c_api.h"

// static event callback wrappers
static void __window_window_close_callback(const event_t &_e) { window.on_window_close_event(_e); }
static void __window_toggle_fullscreen_callback(const event_t &_e) { window.on_toggle_fullscreen_event(_e); }
static void __window_toggle_cursor_callback(const event_t &_e) { window.on_toggle_cursor_event(_e); }
static void __window_toggle_frozen_cursor_callback(const event_t &_e) { window.on_toggle_frozen_cursor_event(_e); }
static void __window_keydown_callback(const event_t &_e) { window.on_keydown_event(_e); }

// glfw callbacks
void __glfw_window_resize_callback(GLFWwindow *_window, int _width, int _height) { window.glfw_window_resize_callback(window.m_window_ptr, _width, _height); }

//
int window_t::init(const char *_name, int _width, int _height)
{
    m_title  = _name;
    int w  = _width == 0 ? 1280 : _width;
    int h = _height == 0 ?  800 : _height;
    m_window_dim = { w, h };
    
	// init GLFW
	//
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); 
	if (!glfwInit()) {
		SYN_ERROR("glfwInit() failed.\n");
		return RETURN_FAILURE;
	}
	
	// desktop resolution (for positioning and size)
	#ifdef _WIN64
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	int x_offset = desktop.right;
	#endif

	glfwDefaultWindowHints();
	
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);	
	glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	
	// create window
	m_window_ptr = glfwCreateWindow(m_window_dim.x, m_window_dim.y, m_title, NULL, NULL);
	if (m_window_ptr) {
		SYN_INFO("GLFW window [%dx%d] created.\n", m_window_dim.x, m_window_dim.y);
	} else {
		SYN_ERROR("GLFW window could not be created.\n");
		return RETURN_FAILURE;
	}

	// pointer storage
	glfwMakeContextCurrent(m_window_ptr);
	//glfwSwapInterval(m_vsync);
	glfwSwapInterval(0);
	glfwSetWindowUserPointer(m_window_ptr, this);
	m_primary_monitor = glfwGetPrimaryMonitor();
	video_mode = glfwGetVideoMode(m_primary_monitor);
	m_screen_dim = { video_mode->width, video_mode->height };
    SYN_INFO("primary monitor size %dx%d.\n", m_screen_dim.x, m_screen_dim.y);

    m_render_dim = m_window_dim;

	// set window size
	if (m_window_dim.x == 0 || m_window_dim.y == 0) {
	    m_window_dim = m_screen_dim;
		m_window_position = { 0, 0 };
	}

	glfwSetWindowSize(m_window_ptr, m_window_dim.x, m_window_dim.y);
	glfwSetWindowPos(m_window_ptr, m_window_position.x, m_window_position.y);

	// window callbacks
	//
	glfwSetFramebufferSizeCallback(m_window_ptr, __glfw_window_resize_callback);
	// callbacks for keyboard and mouse input reside in the InputManager static class
	glfwSetKeyCallback(m_window_ptr, __input_key_callback);
	glfwSetMouseButtonCallback(m_window_ptr, __input_mouse_button_callback);
	glfwSetCursorPosCallback(m_window_ptr, __input_cursor_position_callback);
	glfwSetScrollCallback(m_window_ptr, __input_mouse_scroll_callback);


	// init glew/glad
	//
	#ifdef _WIN64
    if (glewInit() != GLEW_OK) {
    	SYN_ERROR("failed to initialize GLEW.");
    	return RETURN_FAILURE;
    } else {
    	SYN_CORE_TRACE("GLEW initialized.");
    	SYN_CORE_TRACE("OpenGL vendor: ", glGetString(GL_VENDOR));
    	SYN_CORE_TRACE("OpenGL renderer: ", glGetString(GL_RENDERER));
    	SYN_CORE_TRACE("OpenGL version: ", glGetString(GL_VERSION));
    }
	#else
	if (!gladLoadGL(glfwGetProcAddress)) {
		SYN_ERROR("[glad] could not load glfw proc address.\n");
		return RETURN_FAILURE;
	} else {
		SYN_INFO("GLAD initialized.\n");
		SYN_INFO("OpenGL vendor: %s.\n", glGetString(GL_VENDOR));
		SYN_INFO("OpenGL renderer: %s.\n", glGetString(GL_RENDERER));
		SYN_INFO("OpenGL version: %s.\n", glGetString(GL_VERSION));
	}
	#endif

	// register callback for events
	//
	events.register_callback(event_type_t::WINDOW_CLOSE, __window_window_close_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_FULLSCREEN, __window_toggle_fullscreen_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_CURSOR, __window_toggle_cursor_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR, __window_toggle_frozen_cursor_callback);
	events.register_callback(event_type_t::INPUT_KEYDOWN, __window_keydown_callback);
	// set cursor to middle of window
	glfwPollEvents();
	glfwSetCursorPos(m_window_ptr, m_window_dim.x * 0.5f, m_window_dim.y * 0.5f);

	//
	return RETURN_SUCCESS;
}

//
void window_t::destroy()
{
    SYN_INFO("destroying window and GLFW context.\n");
	glfwDestroyWindow(m_window_ptr);
	glfwTerminate();
}

//
void window_t::pre_render()
{
	glfwPollEvents();
	
}

//
void window_t::post_render()
{
    glfwSwapBuffers(m_window_ptr);
    
}

// 
void window_t::update()
{	
    // handle GLFW events
	glfwPollEvents();

	if (m_is_cursor_frozen && glfwGetWindowAttrib(m_window_ptr, GLFW_FOCUSED)) {
	    glfwSetCursorPos(m_window_ptr, m_window_dim.x * 0.5f, m_window_dim.y * 0.5f);
	}

	glfwSwapBuffers(m_window_ptr);
	
}

//
void window_t::center_cursor()
{
	// glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;
	// glfwSetCursorPos(m_window_ptr, floor(half_vp.x), floor(half_vp.y));
	glfwSetCursorPos(m_window_ptr, m_window_dim.x * 0.5f, m_window_dim.y * 0.5f);
}

//
void window_t::set_fullscreen(const bool& _fullscreen)
{
    // fullscreen here refers to maximized, borderless window
	m_is_fullscreen = _fullscreen;

	if (m_is_fullscreen) {
		glfwSetWindowMonitor(m_window_ptr, m_primary_monitor, 0, 0, m_screen_dim.x, m_screen_dim.y, video_mode->refreshRate);
	} else {
		glfwSetWindowMonitor(m_window_ptr, NULL, m_window_position.x, m_window_position.y, m_window_dim.x, m_window_dim.y, 0);
	}
}

//
void window_t::glfw_window_resize_callback(GLFWwindow* _window, int _width, int _height)
{
    (void)_window;
    event_t e;
    e.type = event_type_t::WINDOW_RESIZE;
    e.as.window_resize.width = _width;
    e.as.window_resize.height = _height;
    events.dispatch_event(e);

    e.type = event_type_t::VIEWPORT_RESIZE;
    e.as.viewport_resize.viewport = glm::ivec2(_width, _height);
    e.as.viewport_resize.viewport_f = glm::vec2(_width, _height);
    events.dispatch_event(e);
}

// 
void window_t::on_window_close_event(const event_t &_e)
{
    SYN_INFO("closing window.\n");
    m_to_close_window = true;

}

// 
void window_t::on_toggle_fullscreen_event(const event_t &_e)
{
    set_fullscreen(!m_is_fullscreen);

}

// 
void window_t::on_toggle_cursor_event(const event_t &_e)
{
	if (!m_is_cursor_visible) {
		glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
	    glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

// 
void window_t::on_toggle_frozen_cursor_event(const event_t &_e)
{
    m_is_cursor_frozen = !m_is_cursor_frozen;

    if (!m_is_cursor_frozen) {
	    glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
	    glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

// 
void window_t::on_keydown_event(const event_t &_e)
{
    if (_e.as.keydown.key == (int)m_to_close_key && _e.as.keydown.action == 1) {
        SYN_INFO("exit signal recieved.\n");
        event_t e;
        e.type = event_type_t::WINDOW_CLOSE;
        events.dispatch_event(e);
    }
}
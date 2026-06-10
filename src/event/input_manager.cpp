
#include <string.h>

#include "event/input_manager.h"

#include "utils/log.h"
#include "event/event_handler.h"
#include "event/event.h"
#include "event/key_codes.h"

#include "c_api.h"

// glfw callback wrappers
void __input_key_callback(GLFWwindow *_window_ptr, int _key, int _scancode, int _action, int _mods) { input.key_callback(_window_ptr, _key, _scancode, _action, _mods); }
void __input_mouse_button_callback(GLFWwindow *_window_ptr, int _button, int _action, int _mods) { input.mouse_button_callback(_window_ptr, _button, _action, _mods); } 
void __input_mouse_scroll_callback(GLFWwindow *_window_ptr, double _offset_x, double _offset_y) { input.mouse_scroll_callback(_window_ptr, _offset_x, _offset_y); } 
void __input_mouse_move_callback(GLFWwindow *_window_ptr, double _x, double _y) { input.mouse_move_callback(_window_ptr, _x, _y); } 

// 
void input_handler_t::init()
{
	// reset input maps
	memset(keys_down_state, 0, sizeof(bool) * MAX_KEYS);
	memset(keys_pressed_state, 0, sizeof(bool) * MAX_KEYS);
	memset(mouse_buttons_state, 0, sizeof(bool) * MAX_MOUSE_BUTTONS);
	mouse_position = glm::vec2(0.0, 0.0);
	mouse_scroll_position = glm::vec2(0.0, 0.0);

	SYN_INFO("listening.\n");
}

// 
bool input_handler_t::is_key_down(unsigned int _key)
{
	if (_key >= MAX_KEYS)
		return false;
	return keys_down_state[_key];
}

// 
bool input_handler_t::was_key_pressed(unsigned int _key)
{
    if (_key >= MAX_KEYS)
		return false;
	bool result = keys_pressed_state[_key] && keys_down_state[_key];
	keys_pressed_state[_key] = SYN_KEY_RELEASED;
	return result;
}

// 
bool input_handler_t::is_button_pressed(unsigned int _button)
{
	if (_button >= MAX_MOUSE_BUTTONS)
		return false;
	return mouse_buttons_state[_button];
}

// 
void input_handler_t::key_callback(GLFWwindow *_window_ptr, int _key, int _scancode, int _action, int _mods)
{
    (void)_window_ptr;
    (void)_scancode;
    (void)_mods;
    
	keys_down_state[_key] = _action != SYN_KEY_RELEASED;
	if (_action == SYN_KEY_PRESSED) {
        keys_pressed_state[_key] = SYN_KEY_PRESSED;
	} else {
	    keys_pressed_state[_key] = SYN_KEY_RELEASED;
	}

	event_t e;
	e.type = event_type_t::INPUT_KEYDOWN;
	e.as.keydown.action = _action;
	e.as.keydown.key = _key;
	e.as.keydown.mods = _mods;
	e.as.keydown.focused_window_handle = window_manager.get_focused_window();
	
	events.dispatch_event(e);

}

// 
void input_handler_t::mouse_button_callback(GLFWwindow *_window_ptr, int _button, int _action, int _mods)
{
    (void)_window_ptr;

    mouse_buttons_state[_button] = _action != SYN_MOUSE_BUTTON_RELEASED;

    event_t e;
    e.type = event_type_t::INPUT_MOUSE_BUTTON;
    e.as.mouse_button.button = _button;
    e.as.mouse_button.action = _action;
    e.as.mouse_button.mods = _mods;
    e.as.mouse_button.pos = mouse_position;
    e.as.mouse_button.window_handle = window_manager.get_window_at_pos(mouse_position);

    events.dispatch_event(e);
    
}

// 
void input_handler_t::mouse_scroll_callback(GLFWwindow *_window_ptr, double _offset_x, double _offset_y)
{
    (void)_window_ptr;

	mouse_scroll_position.x = (float)_offset_x;
	mouse_scroll_position.y = (float)_offset_y;

	event_t e;
	e.type = event_type_t::INPUT_MOUSE_SCROLL;
	e.as.mouse_scroll.xoffset = mouse_scroll_position.x;
	e.as.mouse_scroll.yoffset = mouse_scroll_position.y;
	double x, y;
	glfwGetCursorPos(root_window.m_window_ptr, &x, &y);
	e.as.mouse_scroll.window_handle = window_manager.get_window_at_pos(glm::vec2(x, y));

	events.dispatch_event(e);

}

// 
void input_handler_t::mouse_move_callback(GLFWwindow *_window_ptr, double _x, double _y)
{
    (void)_window_ptr;
 
	mouse_position.x = (float)_x;
	mouse_position.y = (float)_y;

	event_t e;
	e.type = event_type_t::INPUT_MOUSE_MOVE;
	e.as.mouse_move.pos = mouse_position;
	e.as.mouse_move.window_handle = window_manager.get_window_at_pos(mouse_position);

	events.dispatch_event(e);

}

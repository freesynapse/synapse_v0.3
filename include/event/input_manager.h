#ifndef __INPUT_MANAGER_H
#define __INPUT_MANAGER_H


#ifndef GLAD_INCLUDED
#include "gl_api.h"
#endif
#include <glm/glm.hpp>

#include "event/key_codes.h"

#define MAX_KEYS            1024
#define MAX_MOUSE_BUTTONS     32

//
struct input_handler_t
{
	void init();

	void key_callback(GLFWwindow *_window_ptr, int _key, int _scancode, int _action, int _mods);
	void mouse_button_callback(GLFWwindow *_window_ptr, int _button, int _action, int _mods);
	void mouse_scroll_callback(GLFWwindow *_window_ptr, double _offset_x, double _offset_y);
	void mouse_move_callback(GLFWwindow *_window_ptr, double _x, double _y);

	// accessors
	bool is_key_down(unsigned int _key);
	bool was_key_pressed(unsigned int _key);
	bool is_button_pressed(unsigned int _button);
	
	bool keys_down_state[MAX_KEYS];
	bool keys_pressed_state[MAX_KEYS];
	bool mouse_buttons_state[MAX_MOUSE_BUTTONS];
	glm::vec2 mouse_position;
	glm::vec2 mouse_scroll_position;
	
};

// callback wrappers
void __input_key_callback(GLFWwindow *_window_ptr, int _key, int _scancode, int _action, int _mods);
void __input_mouse_button_callback(GLFWwindow *_window_ptr, int _button, int _action, int _mods);
void __input_mouse_scroll_callback(GLFWwindow *_window_ptr, double _offset_x, double _offset_y);
void __input_mouse_move_callback(GLFWwindow *_window_ptr, double _x, double _y);

#endif // __INPUT_MANAGER_H

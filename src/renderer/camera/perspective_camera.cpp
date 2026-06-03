
#include "window.h"
#include "event/event.h"
#include "event/key_codes.h"
#include "event/input_manager.h"
#include "utils/math_utils.h"

#include "c_api.h"


// static event callback wrappers
static void __perspective_camera_viewport_resize_callback(const event_t &_e) { perspective_camera.on_viewport_resize(_e); }
static void __perspective_camera_window_resize_callback(const event_t &_e) { perspective_camera.on_window_resize(_e); }
static void __perspective_camera_frozen_cursor_callback(const event_t &_e) { perspective_camera.on_cursor_freeze(_e); }
static void __perspective_camera_mouse_scroll_callback(const event_t &_e) { perspective_camera.on_scroll(_e); }

// 
void perspective_camera_t::init(float _fov_deg, 
                                float _screen_w, 
                                float _screen_h, 
                                float _z_near, 
                                float _z_far)
{
	m_x_angle = 0.0f;
	m_y_angle = 0.0f;

	m_fov = _fov_deg;
	m_aspect_ratio = _screen_w / _screen_h;
	m_z_near = _z_near;
	m_z_far = _z_far;

	m_move_speed   = 15.0f;
	m_zoom_speed   = 5.0f;

	m_prev_mouse_position = { 0.0f, 0.0f };

	m_projection_matrix = glm::perspectiveFov(glm::radians(m_fov), 
											  _screen_w,
											  _screen_h,
											  m_z_near, 
											  m_z_far);

	// register for viewport resize events
	events.register_callback(event_type_t::VIEWPORT_RESIZE, __perspective_camera_viewport_resize_callback);
	events.register_callback(event_type_t::WINDOW_RESIZE, __perspective_camera_window_resize_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR, __perspective_camera_frozen_cursor_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, __perspective_camera_mouse_scroll_callback);

}

// 
void perspective_camera_t::update(float _dt)
{
    // break if input is disabled (i.e. in engine edit mode).
	if (!m_do_update_camera || !window.m_is_cursor_frozen)
		return;

    // get input to adjust position and look-at
	handle_input(_dt);

	// calculate pitch and yaw
	glm::mat4 mat_pitch = glm::rotate(glm::mat4(1.0f), glm::radians(m_y_angle), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 mat_yaw = glm::rotate(glm::mat4(1.0f), glm::radians(m_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	// ordered multiplication
	glm::mat4 mat_rotation = mat_pitch * mat_yaw;
	// add in tranlation
	glm::mat4 mat_translate = glm::translate(glm::mat4(1.0f), -m_position);
	// update view and VP matrices
	m_view_matrix = mat_rotation * mat_translate;
	m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	// update directional vectors
	m_look_at_vector = glm::vec3(m_view_matrix[0][2], m_view_matrix[1][2], m_view_matrix[2][2]);
	m_right_vector = glm::vec3(m_view_matrix[0][0], m_view_matrix[1][0], m_view_matrix[2][0]);
	m_up_vector = glm::normalize(glm::cross(m_look_at_vector, m_right_vector));
	// update forward move vector
	m_forward_vector = glm::cross(m_up_vector, m_right_vector);
}

// 
void perspective_camera_t::handle_input(float _dt)
{
	// update position
	if (input.is_key_down(SYN_KEY_D))
		m_position += m_right_vector * m_move_speed * _dt;
	else if (input.is_key_down(SYN_KEY_A))
		m_position -= m_right_vector * m_move_speed * _dt;
	if (input.is_key_down(SYN_KEY_SPACE))
		m_position += glm::vec3(0.0f, 1.0f, 0.0f) * m_move_speed * _dt;
	else if (input.is_key_down(SYN_KEY_LEFT_SHIFT))
		m_position -= glm::vec3(0.0f, 1.0f, 0.0f) * m_move_speed * _dt;
	if (input.is_key_down(SYN_KEY_W))
		m_position += m_forward_vector * m_move_speed * _dt;
	else if (input.is_key_down(SYN_KEY_S))
		m_position -= m_forward_vector * m_move_speed * _dt;

	// update look-at angles
	double x, y;
	glfwGetCursorPos(window.m_window_ptr, &x, &y);
	glm::vec2 mouse_position = { (float)x, (float)y };

	if (m_first_mouse_input) {
		m_prev_mouse_position = mouse_position;
		m_first_mouse_input = false;
		return;
	}

	if (mouse_position.x != m_prev_mouse_position.x || mouse_position.y != m_prev_mouse_position.y) {
		m_mouse_delta = mouse_position - m_prev_mouse_position;
		m_prev_mouse_position = mouse_position;

		if (glm::length(m_mouse_delta) > 150.0f) {
            return;
		}

		m_mouse_delta *= m_look_at_speed;
		m_x_angle += m_mouse_delta.x;// *aspect_ratio;
		m_y_angle += m_mouse_delta.y;

		// clamp angles
		if (m_x_angle >= 360.0f) m_x_angle -= 360.0f;
		if (m_x_angle < 0.0f)    m_x_angle += 360.0f;
		m_y_angle = clamp(m_y_angle, -90.0f, 90.0f);
	}

}

// 
void perspective_camera_t::on_viewport_resize(const event_t &_e)
{
    glm::vec2 viewport = _e.as.viewport_resize.viewport_f;
    if (viewport.x <= 0 || viewport.y <= 0) return;

    m_aspect_ratio = viewport.x / viewport.y;
    update_projection_matrix();
    m_first_mouse_input = true;
}

// 
void perspective_camera_t::on_window_resize(const event_t &_e)
{
    m_first_mouse_input = true;
}

// 
void perspective_camera_t::on_cursor_freeze(const event_t &_e)
{
    update_view_matrix();
	m_first_mouse_input = true;
}

// 
void perspective_camera_t::on_scroll(const event_t &_e)
{
    //
}


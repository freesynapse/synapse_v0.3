
#include "renderer/camera/orbit_camera.h"
#include "renderer/renderer.h"
#include "event/key_codes.h"
#include "window.h"
#include "event/input_manager.h"
#include "utils/math_utils.h"

#include "c_api.h"


// static event callback wrappers
static void __orbit_camera_window_resize_callback(const event_t &_e) { orbit_camera.on_window_resize(_e); }
static void __orbit_camera_viewport_resize_callback(const event_t &_e) { orbit_camera.on_viewport_resize(_e); }
static void __orbit_camera_frozen_cursor_callback(const event_t &_e) { orbit_camera.on_cursor_freeze(_e); }
static void __orbit_camera_mouse_scroll_callback(const event_t &_e) { orbit_camera.on_scroll(_e); }

// events.register_callback(event_type_t::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(orbit_camera_t::on_event));
	// events.register_callback(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(orbit_camera_t::on_event));
	// events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, SYN_EVENT_MEMBER_FNC(orbit_camera_t::on_event));


// 
void orbit_camera_t::init(float _fov_deg, 
						  float _screen_w, 
						  float _screen_h, 
						  float _z_near, 
						  float _z_far)
{
	m_fov = _fov_deg;
	m_aspect_ratio = _screen_w / _screen_h;
	m_z_near = _z_near;
	m_z_far = _z_far;

	// default for orbit cameras (differs from default of parent PerspectiveCamera)
	m_zoom_speed = 1.0f;

	m_projection_matrix = glm::perspectiveFov(glm::radians(m_fov), 
											  _screen_w,
											  _screen_h,
											  m_z_near, 
											  m_z_far);

	m_prev_mouse_position = { 0.0f, 0.0f };

	// register for viewport resize events
	events.register_callback(event_type_t::VIEWPORT_RESIZE, __orbit_camera_viewport_resize_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR, __orbit_camera_frozen_cursor_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, __orbit_camera_mouse_scroll_callback);
}

// 
void orbit_camera_t::update(float _dt)
{
	// get input to adjust position and look-at
	handle_input(_dt);

	// calculate the up vector
	m_forward_vector = glm::normalize(m_position - m_target_vector);
	// calculate temporary up vector
	if (fabs(m_forward_vector.x) < FLT_EPSILON && fabs(m_forward_vector.z) < FLT_EPSILON) {
		if (m_forward_vector.y > 0)	// looking along pos y-axis
			m_up_vector = { 0, 0, -1 };
		else					// looking along neg y-axis
			m_up_vector = { 0, 0, 1 };
	}
	else
		m_up_vector = { 0, 1, 0 };
	
	// calculate the right vector
	m_right_vector = glm::normalize(glm::cross(m_forward_vector, m_up_vector));
	// recalculate the up vector
	m_up_vector = glm::normalize(glm::cross(m_right_vector, m_forward_vector));

	// update camera position based on rotation angles
	m_position.x = m_radius * sinf(deg_to_rad(m_y_angle)) * cosf(deg_to_rad(m_x_angle));
	m_position.y = m_radius * cosf(deg_to_rad(m_y_angle));
	m_position.z = m_radius * sinf(deg_to_rad(m_y_angle)) * sinf(deg_to_rad(m_x_angle));

	// set view matrix and update VP matrix
	m_view_matrix = glm::lookAt(m_position, m_target_vector, m_up_vector);
	m_view_projection_matrix = m_projection_matrix * m_view_matrix;

}

// 
void orbit_camera_t::handle_input(float _dt)
{
	// break if input is disabled (i.e. in engine edit mode).
	if (!m_do_update_camera)// || !window.m_is_cursor_frozen)
		return;
	
	// update look-at angles
	// glm::vec2 mouse_position = input.mouse_position;
	glm::vec2 mouse_position;
	double x, y;
	glfwGetCursorPos(window.m_window_ptr, &x, &y);
	mouse_position = { (float)x, (float)y };
	
	if (m_first_mouse_input) {
		m_prev_mouse_position = mouse_position;
		m_first_mouse_input = false;
	}

	if ((input.is_button_pressed(SYN_MOUSE_BUTTON_1)) &&	// requires mouse pressed
		(mouse_position.x != m_prev_mouse_position.x || mouse_position.y != m_prev_mouse_position.y)) {

		m_mouse_delta = m_prev_mouse_position - mouse_position;
		m_mouse_delta *= m_orbit_speed;
		//m_mouse_delta *= (m_orbit_speed * _dt);
		m_y_angle += m_mouse_delta.y;
		m_x_angle -= m_mouse_delta.x;
		// clamp angles
		if (m_x_angle >= 360.0f)	m_x_angle -= 360.0f;
		if (m_x_angle < 0.0f)	    m_x_angle += 360.0f;

		m_y_angle = clamp(m_y_angle, 0.1f, 179.9f);
		
	}

	m_prev_mouse_position = mouse_position;

}

// 
void orbit_camera_t::on_viewport_resize(const event_t &_e)
{
    glm::vec2 viewport = _e.as.viewport_resize.viewport_f;
    if (viewport.x <= 0 || viewport.y <= 0) return;

    m_aspect_ratio = viewport.x / viewport.y;
    update_projection_matrix();
    m_first_mouse_input = true;
    
}

// 
void orbit_camera_t::on_window_resize(const event_t &_e)
{    
    m_first_mouse_input = true;
    
}

// 
void orbit_camera_t::on_cursor_freeze(const event_t &_e)
{
	update_view_matrix();
	m_first_mouse_input = true;

}

// 
void orbit_camera_t::on_scroll(const event_t &_e)
{
    float delta = _e.as.mouse_scroll.yoffset * m_zoom_speed;
    m_radius -= delta * m_radius * 0.05f;
	m_radius = clamp(m_radius, 0.01f, m_z_far + 100.0f);

}

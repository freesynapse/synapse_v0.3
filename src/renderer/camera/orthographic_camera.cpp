

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/camera/orthographic_camera.h"
#include "event/event_handler.h"
#include "event/event.h"
#include "event/key_codes.h"

#include "c_api.h"


// static event callback wrappers
static void __orbit_camera_viewport_resize_callback(const event_t &_e) { orbit_camera.on_viewport_resize(_e); }
static void __orbit_camera_frozen_cursor_callback(const event_t &_e) { orbit_camera.on_cursor_freeze(_e); }
static void __orbit_camera_mouse_scroll_callback(const event_t &_e) { orbit_camera.on_mouse_scroll(_e); }

// 
orthographic_camera_t::orthographic_camera_t(float _aspect_ratio, float _zoom_level)
{
	m_aspect_ratio = _aspect_ratio;
	m_zoom_limit = _zoom_level;

	m_zoom_speed = 0.25f;
	m_move_speed = 6.0f;

	m_bounds = { -m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level, m_zoom_level };

	// uses aspect ratio and zoom level
	update_projection_matrix();

	m_view_matrix = glm::mat4(1.0f);

	m_view_projection_matrix = m_projection_matrix * m_view_matrix;

	// register event callbacks
	events.register_callback(event_type_t::VIEWPORT_RESIZE, __orbit_camera_viewport_resize_callback);
	events.register_callback(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR, __orbit_camera_frozen_cursor_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, __orbit_camera_mouse_scroll_callback);	

}

// 
void orthographic_camera_t::set_projection_matrix(float _left, float _right, float _bottom, float _top)
{
	m_projection_matrix = glm::ortho(_left, _right, _bottom, _top, -1.0f, 1.0f);
}

// 
void orthographic_camera_t::update_view_matrix()
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) * 
						  glm::rotate(glm::mat4(1.0f), glm::radians(m_theta), glm::vec3(0.0f, 0.0f, 1.0f));

	//view_matrixInverted = transform;
	m_view_matrix = glm::inverse(transform);
	m_view_projection_matrix = m_projection_matrix * m_view_matrix;
}

// 
void orthographic_camera_t::update_projection_matrix()
{
	m_projection_matrix = glm::ortho(-m_aspect_ratio * m_zoom_level, 
	                                  m_aspect_ratio * m_zoom_level,
	                                 -m_zoom_level,
	                                  m_zoom_level,
									 -1.0f, 1.0f);

	m_view_projection_matrix = m_projection_matrix * m_view_matrix;										
}

// 
void orthographic_camera_t::reset()
{
	m_position = glm::vec3(0.0f);
	m_theta = 0.0f;
	m_zoom_level = 1.0f;

	update_projection_matrix();
	update_view_matrix();
}

// 
void orthographic_camera_t::update(float _dt)
{
	// only update if in engine camera mode (i.e. not edit mode).
	// -- not true for orthographic cameras
	//if (!m_updateCamera)
	//	return;
		
	glm::vec3 prevPos = m_position;
	float prevTheta = m_theta;
	float adj_speed = m_move_speed * _dt * m_zoom_level;

	// update position
	if (input.is_key_down(SYN_KEY_D)) {
		m_position.x += cos(glm::radians(m_theta)) * adj_speed;
		m_position.y += sin(glm::radians(m_theta)) * adj_speed;
	}
	else if (input.is_key_down(SYN_KEY_A)) {
		m_position.x -= cos(glm::radians(m_theta)) * adj_speed;
		m_position.y -= sin(glm::radians(m_theta)) * adj_speed;
	}
	
	if (input.is_key_down(SYN_KEY_W)) {
		m_position.x += -sin(glm::radians(m_theta)) * adj_speed;
		m_position.y +=  cos(glm::radians(m_theta)) * adj_speed;
	}
	else if (input.is_key_down(SYN_KEY_S)) {
		m_position.x -= -sin(glm::radians(m_theta)) * adj_speed;
		m_position.y -=  cos(glm::radians(m_theta)) * adj_speed;
	}

	// update rotation
	if (input.is_key_down(SYN_KEY_Q))
		m_theta += m_rotation_speed * _dt;
	if (input.is_key_down(SYN_KEY_E))
		m_theta -= m_rotation_speed * _dt;
	
	// rotation limits
	if (m_theta > 180.0f)
		m_theta -= 360.0f;
	else if (m_theta <= -180.0f)
		m_theta += 360.0f;

	// reset?
	if (input.is_key_down(SYN_KEY_R))
		reset();


	// update the view matrix if position has changed
	if ((prevPos != m_position) || (prevTheta != m_theta))
		update_view_matrix();

}

// 
void orthographic_camera_t::on_viewport_resize(const event_t &_e)
{
    glm::vec2 viewport = _e.as.viewport_resize.fviewport;
    if (viewport.x <= 0 || viewport.y <= 0) return;

    m_aspect_ratio = viewport.x / viewport.y;
    m_bounds = { -m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level, m_zoom_level };
    update_projection_matrix();
    
}

// 
void orthographic_camera_t::on_cursor_freeze(const event_t &_e)
{
    update_projection_matrix();
}

// 
void orthographic_camera_t::on_mouse_scroll(const event_t &_e)
{
    if (m_zoom_amplifier > 1.0f || m_zoom_amplifier < 1.0f) {
        m_zoom_level *= pow(m_zoom_amplifier, _e.as.mouse_scroll.yoffset);// * zoom_speed);
    } else {
	    m_zoom_level -= _e.as.mouse_scroll.yoffset * m_zoom_speed;
	}
 
	m_zoom_level = std::max(m_zoom_limit, m_zoom_level);
	m_bounds = { -m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level, m_zoom_level };				
	update_projection_matrix();
    
}


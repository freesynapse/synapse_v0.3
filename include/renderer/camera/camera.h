#ifndef __CAMERA_H
#define __CAMERA_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "event/event.h"


enum class camera_input_action
{
	NO_ACTION,
	MOVE_UP, MOVE_DOWN, MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT,
	STRAFE_LEFT, STRAFE_RIGHT,
	INCREASE_THETA, DECREASE_THETA,
	INCREASE_PHI, DECREASE_PHI,
	INCREASE_RADIUS, DECREASE_RADIUS,
};


class camera_t
{
public:
	virtual ~camera_t() = default;

	virtual void update_view_matrix() {};
	virtual void reset() {};
	
	virtual void update(float _dt) {}
	virtual void update_projection_matrix() { m_projection_matrix = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_z_near, m_z_far); }
	virtual void on_input(camera_input_action _action, float _dt) {}
	virtual void on_input(float _dx, float _dy, float _dt) {}
	virtual void on_input(float _dt) {}
	// virtual void on_event(event_t *_e) = 0;

	// accessors
	virtual void set_update_mode(bool _update) { m_do_update_camera = _update; }

	// virtual void setProjectionMatrix(float _fov, float _width, float _height, float _z_near, float _z_far)
	// { projection_matrix = glm::perspective(glm::radians(_fov), _width / _height, _z_near, _z_far); }
	virtual void set_projection_matrix(const glm::mat4& _mat) { m_projection_matrix = _mat; }
	virtual void set_projection_matrix(float _left, float _right, float _bottom, float _top) {}
	/* FOV (in degrees) */
	virtual void set_fov(float _fov) { m_fov = _fov; update_projection_matrix(); }
	virtual void set_view_distance(float _znear, float _zfar) { m_z_near = _znear; m_z_far = _zfar; update_projection_matrix(); }

	virtual const glm::mat4& get_projection_matrix() const { return m_projection_matrix; }
	virtual const glm::mat4& get_view_matrix() const { return m_view_matrix; }
	virtual glm::mat4 get_inverted_view_matrix() { return glm::inverse(m_view_matrix); }
	virtual const glm::mat4& get_view_projection_matrix() const { return m_view_projection_matrix; }
	virtual const glm::vec3& get_position() const { return m_position; }
	virtual void set_position(const glm::vec3& _v) { m_position = _v; }
	/* FOV (in degrees) */
	virtual float get_fov() const { return m_fov; }
	virtual float get_z_near() const { return m_z_near; }
	virtual float get_z_far() const { return m_z_far; }
	virtual float get_aspect_ratio() const { return m_aspect_ratio; }
	virtual void set_aspect_ratio(float _aspect_ratio) { m_aspect_ratio = _aspect_ratio; }
	virtual const glm::vec3& get_look_at_vector() const { return m_look_at_vector; }
	/* Speed variables */
	float get_zoom_speed() const  { return m_zoom_speed; 	}
	void set_zoom_speed(float _s) { m_zoom_speed = _s; 	}
	float get_move_speed() const  { return m_move_speed;   }
	void set_move_speed(float _s) { m_move_speed = _s; 	}

	void set_active(bool _do_update_camera) { m_do_update_camera = _do_update_camera; }
	void set_interactive(bool _interactive) { m_is_interaction_active = _interactive; }
	
// protected:
	glm::mat4 m_projection_matrix = glm::mat4(1.0f);
	glm::mat4 m_view_matrix = glm::mat4(1.0f);
	glm::mat4 m_view_projection_matrix = glm::mat4(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);

	float m_fov = 65.0f;
	float m_aspect_ratio = 16.0f / 9.0f;
	float m_z_near = 0.1f;
	float m_z_far = 1000.0f;

	float m_move_speed   = 15.0f;	// PerspectiveCamera defaults
	float m_zoom_speed   = 5.0f;

	glm::vec3 m_look_at_vector = glm::vec3(0.0f);

	bool m_do_update_camera = true;
	bool m_first_mouse_input = true;
	bool m_is_interaction_active = false;   // integration of input with the window_manager
};


#endif // __CAMERA_H
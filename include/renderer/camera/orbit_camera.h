#ifndef __ORBIT_CAMERA_H
#define __ORBIT_CAMERA_H

#include <glm/gtc/quaternion.hpp>

#include "renderer/camera/perspective_camera.h"
#include "event/event.h"


// 
class orbit_camera_t : public perspective_camera_t
{
public:
    orbit_camera_t() = default;
	void init(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far);

	virtual void update(float _dt) override;
	virtual void handle_input(float _dt) override;

	void on_window_resize(const event_t &_e);
	void on_viewport_resize(const event_t &_e);
	void on_cursor_freeze(const event_t &_e);
	void on_mouse_scroll(const event_t &_e);
	void on_mouse_move(const event_t &_e);
	void on_mouse_button(const event_t &_e);
	
	// accessors
	/* Rotation speed, default 10.0f */
	inline void set_orbit_speed(float _s) { m_orbit_speed = _s; }
	/* Distance from target, initial value 10.0f */
	inline void set_radius(float _r) { m_radius = _r; }
	inline float get_radius() const { return m_radius; }

// private:
	glm::vec2 m_prev_mouse_position = { 0.0f, 0.0f };
	glm::vec2 m_mouse_delta   	    = { 0.0f, 0.0f };

	float m_radius		    = 10.0f;	// distance from target
	//float m_zoomSpeed 	=  1.0f;
	float m_orbit_speed 	=  0.1f;

	//float m_xAngle      = 0.0f;		// theta : x/z rotation angle (degrees)
	//float m_yAngle		= 90.0f;	// phi   :   y rotation angle (degrees)

	glm::vec3 m_target_vector 	= { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_up_vector 		= { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_right_vector  	= { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_forward_vector  = { 0.0f, 0.0f, 0.0f };
	// look-at vector is inherited from Camera

};


#endif // __ORBIT_CAMERA_H

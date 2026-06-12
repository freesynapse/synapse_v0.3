#ifndef __PERSPECTIVE_CAMERA_H
#define __PERSPECTIVE_CAMERA_H

#include <glm/gtc/quaternion.hpp>

#include "renderer/camera/camera.h"
#include "event/event.h"

// 
class perspective_camera_t : public camera_t
{
public:
	perspective_camera_t() = default;
	perspective_camera_t(const perspective_camera_t&) = default;

	void init(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far);

	virtual void update(float _dt) override;
	virtual void handle_input(float _dt);
	virtual void update_view_matrix() override;

	void on_viewport_resize(const event_t &_e);
	void on_window_resize(const event_t &_e);
	void on_cursor_freeze(const event_t &_e);
	void on_mouse_move(const event_t &_e);
	void on_keydown(const event_t &e);

	inline float get_x_angle() const  	 { return m_x_angle;   	}
	inline void set_x_angle(float _x) 	 { m_x_angle = _x;   	}
	inline float get_y_angle() const  	 { return m_y_angle;    }
	inline void set_y_angle(float _y) 	 { m_y_angle = _y;   	}
	/* default = 0.05f */
	inline float get_look_at_speed() const  { return m_look_at_speed; }
	/* default = 0.05f */
	inline void set_look_at_speed(float _s) { m_look_at_speed = _s; }
	virtual inline const glm::vec2& get_mouse_delta()       const  { return m_mouse_delta;    }
	virtual inline const glm::vec3& get_look_at_vector()    const override { return m_look_at_vector;  }
	virtual inline const glm::vec3& get_forward_vector()    const  { return m_forward_vector; }
	virtual inline const glm::vec3& get_right_vector()      const  { return m_right_vector;   }
	virtual inline const glm::vec3& get_up_vector() 	    const  { return m_up_vector; 	    }

// private:


// protected:
	glm::vec2 m_prev_mouse_position  = { 0.0f, 0.0f };
	glm::vec2 m_mouse_delta 	     = { 0.0f, 0.0f };

	float m_look_at_speed = 0.05f;

	float m_x_angle = 0.0f;
	float m_y_angle = 0.0f;

	glm::vec3 m_up_vector 		= { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_right_vector 	= { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_forward_vector  = { 0.0f, 0.0f, 0.0f };
	
	// the look-at vector is inherited from Camera
	//glm::vec3 look_at_vector = glm::vec3(0.0f);
};


#endif // __PERSPECTIVE_CAMERA_H

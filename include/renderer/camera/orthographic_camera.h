#ifndef __ORTHOPGRAPHIC_CAMERA_H
#define __ORTHOPGRAPHIC_CAMERA_H


#include "renderer/camera/camera.h"


// 
struct orthographic_camera_bounds_t
{
	float left, right;
	float bottom, top;

	float get_width()  { return right - left; }
	float get_height() { return top - bottom; }
};

// 
class orthographic_camera_t : public camera_t
{
public:
	// default constructor -- with zoom
	orthographic_camera_t() = default;
	orthographic_camera_t(float _aspect_ratio, float _zoom_level=10.0f);
	~orthographic_camera_t() {}

	// Required in every inheritance of the base Camera class:
	// ImGui calls camera_ptr->updateProjectionMatrix() on viewport resize.
	virtual void update_projection_matrix() override;

	// specific for orthographic camera
	virtual void update_view_matrix() override;

	// usual overrides
	virtual void update(float _dt) override;
	virtual void reset() override;
	void on_viewport_resize(const event_t &_e);
	void on_cursor_freeze(const event_t &_e);
	void on_scroll(const event_t &_e);

	// accessors
	inline void set_projection_matrix(float _left, float _right, float _bottom, float _top) override;
	inline void set_position(const glm::vec3& _vpos) override { m_position = _vpos; update_view_matrix(); }
	inline void set_rotation(const float& _theta) { m_theta = _theta; update_view_matrix(); }
	inline float get_theta() const { return m_theta; }
	inline void set_zoom_level(float _zoom_level) { m_zoom_level = _zoom_level; }
	inline float get_zoom_level() { return m_zoom_level; }
	inline void set_zoom_limit(float _zoom_limit) { m_zoom_limit = _zoom_limit; }
	inline float get_zoom_limit() { return m_zoom_limit; }
	inline void set_zoom_amplifier(float _zoom_amp) { m_zoom_amplifier = _zoom_amp; }
	inline float get_zoom_amplifier() { return m_zoom_amplifier; }

	inline const orthographic_camera_bounds_t& getBounds() const { return m_bounds; }

// private:
	float m_theta = 0.0f;
	float m_zoom_level = 1.0f;
	float m_zoom_limit = 0.05f;
	float m_zoom_amplifier = 1.0f;

	orthographic_camera_bounds_t m_bounds;

	float m_rotation_speed = 180.0f;

};


#endif // __ORTHOPGRAPHIC_CAMERA_H

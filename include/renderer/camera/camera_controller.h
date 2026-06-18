#ifndef __CAMERA_CONTROLLER_H
#define __CAMERA_CONTROLLER_H

#include "renderer/camera/orbit_camera.h"
#include "renderer/camera/perspective_camera.h"

// 
enum class camera_mode_t {
    ORBIT,
    PERSPECTIVE,
};

// 
class camera_controller_t
{
public:
    void init(orbit_camera_t *_orbit_cam, perspective_camera_t *_fps_cam);
    void update(float _dt);
    void set_mode(camera_mode_t _mode);
    void toggle_mode();

    void enable();
    void disable();
    
    void update_projection_matrix();
    void set_aspect_ratio(float _ar);
    
    camera_mode_t get_mode() { return m_mode; }
    bool is_orbit() { return m_mode == camera_mode_t::ORBIT; }
    bool is_pesepective() { return m_mode == camera_mode_t::PERSPECTIVE; }

    const glm::vec3 &get_position();
    const glm::mat4 &get_view_matrix();
    const glm::mat4 &get_projection_matrix();
    const glm::mat4 &get_view_projection_matrix();
    float get_z_near();
    float get_z_far();

private:
    void sync_perspective_from_orbit();
    void sync_orbit_from_perspective();
    
    camera_mode_t m_mode = camera_mode_t::ORBIT;
    orbit_camera_t *m_orbit_ptr = nullptr;
    perspective_camera_t *m_perspective_ptr = nullptr;

};


#endif // __CAMERA_CONTROLLER_H


#include <glm/gtc/matrix_transform.hpp>

#include "renderer/camera/camera_controller.h"

#include "c_api.h"


// 
void camera_controller_t::init(orbit_camera_t *_orbit_cam, perspective_camera_t *_fps_cam)
{
    m_orbit_camera_ptr = _orbit_cam;
    m_fps_camera_ptr = _fps_cam;

    // initialize state(s)
    orbit.current_distance = orbit.target_distance;
    orbit.current_yaw = orbit.target_yaw;
    orbit.current_pitch = orbit.target_pitch;
    orbit.current_focus = orbit.target_focus;
    
    fps.current_position = fps.target_position;
    fps.current_yaw = fps.target_yaw;
    fps.current_pitch = fps.target_pitch;
     
}

// 
void camera_controller_t::update(float dt)
{
    if (m_mode == camera_mode_t::ORBIT) {
        handle_orbit_input(dt);
          
          // Smooth interpolation
          orbit.current_distance = lerp(orbit.current_distance, orbit.target_distance, m_orbit_smooth_factor);
          orbit.current_yaw = lerp(orbit.current_yaw, orbit.target_yaw, m_orbit_smooth_factor);
          orbit.current_pitch = lerp(orbit.current_pitch, orbit.target_pitch, m_orbit_smooth_factor);
          orbit.current_focus = lerp(orbit.current_focus, orbit.target_focus, m_orbit_smooth_factor);
          
          // Update orbit camera
          //m_orbit_camera_ptr->set_distance(orbit.current_distance);
          //m_orbit_camera_ptr->set_yaw(orbit.current_yaw);
          //m_orbit_camera_ptr->set_pitch(orbit.current_pitch);
          //m_orbit_camera_ptr->set_focus_point(orbit.current_focus);
               
    }
    else if (m_mode == camera_mode_t::PERSPECTIVE) {
        
    }
    
}

// 
void camera_controller_t::switch_mode(camera_mode_t new_mode)
{
    
}

// 

void camera_controller_t::handle_orbit_input(float dt)
{
    
}

// 
void camera_controller_t::handle_fps_input(float dt)
{
    
}

// 

glm::mat4 camera_controller_t::get_view_matrix()
{
    return glm::mat4(1.0f);

}

// 
glm::mat4 camera_controller_t::get_projection_matrix()
{
    return glm::mat4(1.0f);

}

// 
glm::mat4 camera_controller_t::get_view_projection_matrix()
{
    return glm::mat4(1.0f);

}

// 
float camera_controller_t::lerp(float _a, float _b, float _t) { return _a + (_b - _a) * _t; }
glm::vec3 camera_controller_t::lerp(const glm::vec3 &_a, const glm::vec3 &_b, float _t) { return _a + (_b - _a) * _t; }

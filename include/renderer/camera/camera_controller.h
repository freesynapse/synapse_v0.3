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
    void init(orbit_camera_t* orbit_cam, perspective_camera_t* fps_cam);
    void update(float dt);
    void switch_mode(camera_mode_t new_mode);
    
    void handle_orbit_input(float dt);
    void handle_fps_input(float dt);
    
    glm::mat4 get_view_matrix();
    glm::mat4 get_projection_matrix();
    glm::mat4 get_view_projection_matrix();

public:
    camera_mode_t m_mode = camera_mode_t::ORBIT;

    // smoothing parameters
    float m_orbit_smooth_factor = 0.15f;  // [0.1 .. 0.3], lower: smoother
    float m_fps_smooth_factor = 0.2f;
    float m_zoom_smooth_factor = 0.1f;

    // orbit camera state
    struct {
        float target_distance = 5.0f;
        float current_distance = 5.0f;
        float target_yaw = 0.0f;
        float current_yaw = 0.0f;
        float target_pitch = 0.0f;
        float current_pitch = 0.0f;
        glm::vec3 target_focus = glm::vec3(0.0f);
        glm::vec3 current_focus = glm::vec3(0.0f);
        
        float rotation_speed = 0.3f;
        float zoom_speed = 1.0f;
        float pan_speed = 0.01f;
        float min_distance = 1.0f;
        float max_distance = 50.0f;
    } orbit;
    
    // fps camera state
    struct {
        glm::vec3 target_position = glm::vec3(0.0f, 2.0f, 5.0f);
        glm::vec3 current_position = glm::vec3(0.0f, 2.0f, 5.0f);
        float target_yaw = -90.0f;
        float current_yaw = -90.0f;
        float target_pitch = 0.0f;
        float current_pitch = 0.0f;
        
        float move_speed = 5.0f;
        float sprint_multiplier = 2.0f;
        float look_sensitivity = 0.1f;
    } fps;

private:
    orbit_camera_t *m_orbit_camera_ptr = nullptr;
    perspective_camera_t *m_fps_camera_ptr = nullptr;

    float lerp(float _a, float _b, float _t);
    glm::vec3 lerp(const glm::vec3 &_a, const glm::vec3 &_b, float _t);
    
};


#endif // __CAMERA_CONTROLLER_H


#include "renderer/camera/camera_controller.h"

#include "c_api.h"


// 
void camera_controller_t::init(orbit_camera_t* _orbit_cam, perspective_camera_t* _perspective_cam)
{
    m_orbit_ptr = _orbit_cam;
    m_perspective_ptr = _perspective_cam;

    m_mode = camera_mode_t::ORBIT;
    m_orbit_ptr->set_active(true);
    m_perspective_ptr->set_active(false);
    
}

// 
void camera_controller_t::update(float _dt)
{
    if (m_mode == camera_mode_t::ORBIT) {
        m_orbit_ptr->update(_dt);
    } else {
        m_perspective_ptr->update(_dt);
    }
}

// 
void camera_controller_t::set_mode(camera_mode_t _mode)
{
    if (m_mode == _mode) return;
    
    if (_mode == camera_mode_t::ORBIT) {
        sync_orbit_from_perspective();
        m_orbit_ptr->set_active(true);
        m_perspective_ptr->set_active(false);

        m_orbit_ptr->m_first_mouse_input = true;    

    } else {
        sync_perspective_from_orbit();
        m_orbit_ptr->set_active(false);
        m_perspective_ptr->set_active(true);

        m_perspective_ptr->m_first_mouse_input = true;    
    }

    event_t e;
    e.type = event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR;
    events.dispatch_event(e);
    events.flush_event_type(event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR);
    
    m_mode = _mode;
    renderer.show_notification(m_mode == camera_mode_t::ORBIT ? "Camera: Orbit" : "Camera: Perspective");
    
}

// 
void camera_controller_t::toggle_mode()
{
    set_mode(m_mode == camera_mode_t::ORBIT ? camera_mode_t::PERSPECTIVE : camera_mode_t::ORBIT);
    
}

void camera_controller_t::focus_on(const glm::vec3 &_target, float _target_distance)
{
    if (m_mode == camera_mode_t::ORBIT) m_orbit_ptr->focus_on(_target, _target_distance);
    
}

// 
void camera_controller_t::enable()
{
    if (m_mode == camera_mode_t::ORBIT) { m_orbit_ptr->set_active(true); } 
    else { m_perspective_ptr->set_active(true); }
}

// 
void camera_controller_t::disable()
{
    if (m_mode == camera_mode_t::ORBIT) { m_orbit_ptr->set_active(false); }
    else { m_perspective_ptr->set_active(false); }
}


// 
void camera_controller_t::update_projection_matrix()
{
    m_orbit_ptr->update_projection_matrix();
    m_perspective_ptr->update_projection_matrix();

}

// 
void camera_controller_t::set_aspect_ratio(float _ar)
{
    m_orbit_ptr->set_aspect_ratio(_ar);
    m_perspective_ptr->set_aspect_ratio(_ar);
    
}

// 
const glm::vec3 &camera_controller_t::get_position()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_position() : 
                m_perspective_ptr->get_position();
}

// 
const glm::mat4 &camera_controller_t::get_view_matrix()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_view_matrix() : 
                m_perspective_ptr->get_view_matrix();
    
}

// 
const glm::mat4 &camera_controller_t::get_projection_matrix()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_projection_matrix() : 
                m_perspective_ptr->get_projection_matrix();
}

// 
const glm::mat4 &camera_controller_t::get_view_projection_matrix()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_view_projection_matrix() : 
                m_perspective_ptr->get_view_projection_matrix();
}

// 
float camera_controller_t::get_z_near()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_z_near() : 
                m_perspective_ptr->get_z_near();
}

// 
float camera_controller_t::get_z_far()
{
    return m_mode == camera_mode_t::ORBIT ? 
                m_orbit_ptr->get_z_far() : 
                m_perspective_ptr->get_z_far();
}


// 
void camera_controller_t::sync_perspective_from_orbit()
{
    glm::vec3 eye = m_orbit_ptr->get_position();
    m_perspective_ptr->set_position(eye);

    glm::vec3 forward = glm::normalize(m_orbit_ptr->m_focus_point - eye);
    float xangle =  glm::degrees(atan2f(forward.x, -forward.z));
    float yangle = -glm::degrees(asinf(glm::clamp(forward.y, -1.0f, 1.0f)));

    m_perspective_ptr->set_position(eye);
    m_perspective_ptr->set_x_angle(xangle);
    m_perspective_ptr->set_y_angle(yangle);
    m_perspective_ptr->update_view_matrix();

}

// 
void camera_controller_t::sync_orbit_from_perspective()
{
    glm::vec3 pos = m_perspective_ptr->get_position();
    glm::vec3 forward = glm::normalize(-m_perspective_ptr->get_look_at_vector());

    m_orbit_ptr->m_focus_point = pos + forward * m_orbit_ptr->m_distance;

    glm::vec3 to_eye = glm::normalize(pos - m_orbit_ptr->m_focus_point);
    float xangle = glm::degrees(atan2f(to_eye.z, to_eye.x));
    float yangle = glm::degrees(acosf(glm::clamp(to_eye.y, -1.0f, 1.0f)));

    m_orbit_ptr->set_x_angle(xangle);
    m_orbit_ptr->set_y_angle(yangle);
    m_orbit_ptr->update_view_matrix();

}


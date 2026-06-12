
#include "renderer/camera/orbit_camera.h"
#include "event/key_codes.h"
#include "utils/math_utils.h"

#include "c_api.h"


// static event callback wrappers
static void __orbit_camera_window_resize_callback(const event_t &_e) { orbit_camera.on_window_resize(_e); }
static void __orbit_camera_viewport_resize_callback(const event_t &_e) { orbit_camera.on_viewport_resize(_e); }
static void __orbit_camera_frozen_cursor_callback(const event_t &_e) { orbit_camera.on_cursor_freeze(_e); }
static void __orbit_camera_mouse_scroll_callback(const event_t &_e) { orbit_camera.on_mouse_scroll(_e); }
static void __orbit_camera_mouse_move_callback(const event_t &_e) { orbit_camera.on_mouse_move(_e); }
static void __orbit_camera_mouse_button_callback(const event_t &_e) { orbit_camera.on_mouse_button(_e); }

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
	events.register_callback(event_type_t::WINDOW_RESIZE, __orbit_camera_window_resize_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_SCROLL, __orbit_camera_mouse_scroll_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_BUTTON, __orbit_camera_mouse_button_callback);
	events.register_callback(event_type_t::INPUT_MOUSE_MOVE, __orbit_camera_mouse_move_callback);

	double x, y;
    glfwGetCursorPos(root_window.m_window_ptr, &x, &y);
    m_prev_mouse_position = { (float)x, (float)y };
   	
}

// 
void orbit_camera_t::update(float _dt)
{
    update_view_matrix();
}

// 
void orbit_camera_t::update_view_matrix()
{
    // update camera position based on rotation angles
    m_position.x = m_distance * sinf(deg_to_rad(m_y_angle)) * cosf(deg_to_rad(m_x_angle));
    m_position.y = m_distance * cosf(deg_to_rad(m_y_angle));
    m_position.z = m_distance * sinf(deg_to_rad(m_y_angle)) * sinf(deg_to_rad(m_x_angle));
    m_position  += m_focus_point;  // offset by focus point

    // forward vector (eye -> focus)
    m_forward_vector = glm::normalize(m_position - m_focus_point);

    // up vector
    if (fabs(m_forward_vector.x) < FLT_EPSILON && fabs(m_forward_vector.z) < FLT_EPSILON)
        m_up_vector = m_forward_vector.y > 0 ? glm::vec3(0,0,-1) : glm::vec3(0,0,1);
    else
        m_up_vector = glm::vec3(0, 1, 0);

    m_right_vector = glm::normalize(glm::cross(m_forward_vector, m_up_vector));
    m_up_vector    = glm::normalize(glm::cross(m_right_vector, m_forward_vector));

    m_view_matrix            = glm::lookAt(m_position, m_focus_point, m_up_vector);
    m_view_projection_matrix = m_projection_matrix * m_view_matrix;
}

//
void orbit_camera_t::on_viewport_resize(const event_t &_e)
{
    // if the viewport has been set to a window, then update of the
    // projection matrix is handled by window_t::resize_viewport().
    if (window_manager.has_viewport_window()) return;
    
    glm::vec2 viewport = _e.as.viewport_resize.fviewport;
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
	// update_view_matrix();
	// m_first_mouse_input = true;

}

//
void orbit_camera_t::on_mouse_scroll(const event_t &_e)
{
    window_handle_t vp = window_manager.get_viewport_window_handle();
    if (vp.id == 0 || vp.id != _e.as.mouse_scroll.window_handle.id) return;
    
    float delta = _e.as.mouse_scroll.yoffset * m_zoom_speed;
    m_distance += delta * m_distance * 0.05f;
	m_distance = clamp(m_distance, 0.01f, m_z_far + 100.0f);

}

// 
void orbit_camera_t::on_mouse_move(const event_t &_e)
{
    glm::vec2 mouse_position = _e.as.mouse_move.pos;

    #if 0
    static bool once = false;
    if (!once) {
        once = true;
        SYN_INFO("ORBIT move: active=%d interaction=%d pos=(%.1f, %.1f) prev=(%.1f, %.1f)\n",
            m_do_update_camera, m_is_interaction_active,
            mouse_position.x, mouse_position.y,
            m_prev_mouse_position.x, m_prev_mouse_position.y);
    }
    #endif
    if (!m_do_update_camera || !m_is_interaction_active) {
        m_prev_mouse_position = mouse_position;
        return;
    }

    #if 0
    SYN_INFO("passed the m_do_update_camera check..\n");
    #endif

    if (m_first_mouse_input) {
        m_prev_mouse_position = mouse_position;
        m_first_mouse_input = false;
    }

    m_mouse_delta = m_prev_mouse_position - mouse_position;
	m_mouse_delta *= m_orbit_speed;
	m_y_angle += m_mouse_delta.y;
	m_x_angle -= m_mouse_delta.x;

	// clamp angles
	if (m_x_angle >= 360.0f)	m_x_angle -= 360.0f;
	if (m_x_angle < 0.0f)	    m_x_angle += 360.0f;
	m_y_angle = clamp(m_y_angle, 0.1f, 179.9f);
    
	m_prev_mouse_position = mouse_position;
    
}

// 
void orbit_camera_t::on_mouse_button(const event_t &_e)
{
    if (_e.as.mouse_button.button != SYN_MOUSE_BUTTON_1) return;

    window_handle_t vp = window_manager.get_viewport_window_handle();
    if (vp.id == 0) return;
    
    if (_e.as.mouse_button.action == SYN_MOUSE_BUTTON_PRESSED) {
        if (_e.as.mouse_button.window_handle.id != vp.id) return;
        m_is_interaction_active = true;
    } else {
        m_is_interaction_active = false;
    }
}


#ifndef __EDITOR_H
#define __EDITOR_H

#include "renderer/entity/entity_types.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/UI/window/window_types.h"
#include "renderer/UI/ui_types.h"
#include "event/event.h"

// 
class editor_t
{
public:
    friend class renderer_t;
    friend class renderer_2d_t;

public:
    void init();

    // 
    void save_scene(const std::string &_path);
    void load_scene(const std::string &_path);

    // 
    void on_keydown_event(const event_t &_e);

    // 
    void toggle_create_menu(const glm::vec2 &_pos);
    void hide_create_menu();

    // 
    void open_texture_select();
    void assign_texture_to_selected(const std::string &_name);
    void set_texture_select_preview(const std::string &_name);

    // 
    entity_handle_t create_primitive(primitive_type_t _type);
    entity_handle_t pick_entity(const glm::vec2 &_screen_pos);
    ui_transform_axis_t pick_ui_transform_axis(const glm::vec2 &_screen_pos);
    void begin_ui_transform_drag();
    void update_ui_transform_drag(glm::vec2 &_screen_pos);

    // 
    void set_create_window_handle(window_handle_t _handle) { m_create_window_handle = _handle; }
    void set_texture_select_window_handle(window_handle_t _handle) { m_texture_select_window_handle = _handle; }
    void set_hovered_ui_transform_axis(ui_transform_axis_t _axis) { m_hovered_ui_transform_axis = _axis; }
    void set_grabbed_ui_transform_axis(ui_transform_axis_t _axis) { m_grabbed_ui_transform_axis = _axis; }
    void set_ui_transform_mode(ui_transform_mode_t _mode) { m_ui_transform_mode = _mode; }

    window_handle_t get_create_window_handle() { return m_create_window_handle; }
    window_handle_t get_texture_select_window_handle() { return m_texture_select_window_handle; }
    ui_transform_axis_t get_hovered_ui_transform_axis() { return m_hovered_ui_transform_axis; }
    ui_transform_axis_t get_grabbed_ui_transform_axis() { return m_grabbed_ui_transform_axis; }
    ui_transform_mode_t get_ui_transform_mode() { return m_ui_transform_mode; }

private:
    window_handle_t m_create_window_handle = { 0 };
    window_handle_t m_texture_select_window_handle = { 0 };


    // ui transform members
    // 
    // set each frame in window_manager_t::on_mouse_move_event()/::on_mouse_button_event()
    ui_transform_axis_t m_hovered_ui_transform_axis = ui_transform_axis_t::NONE;
    ui_transform_axis_t m_grabbed_ui_transform_axis = ui_transform_axis_t::NONE;

    glm::vec2 m_drag_start_screen;
    glm::vec3 m_drag_start_world;
    glm::vec2 m_drag_origin_ss;

    ui_transform_mode_t m_ui_transform_mode = ui_transform_mode_t::TRANSLATE;

    glm::vec2 m_drag_start_vec;
    
    glm::quat m_drag_start_quat;    // rotation
    glm::vec3 m_drag_start_scale;   // scaling

};



#endif // __EDITOR_H

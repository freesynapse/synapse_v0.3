#ifndef __EDITOR_H
#define __EDITOR_H

#include "renderer/entity/entity_types.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/UI/window/window_types.h"
#include "renderer/UI/ui_types.h"
#include "event/event.h"
#include "editor_command.h"

// 
class editor_t
{
public:
    friend class renderer_t;
    friend class renderer_2d_t;

public:
    void init();
    void shutdown() {}
    
    // 
    void save_scene(const std::string &_path);
    void load_scene(const std::string &_path);
    entity_handle_t create_entity_from_asset(const std::string &_asset_name, const glm::vec3 &_position, const std::string &_material_name);
    entity_handle_t create_entity_from_primitive(primitive_type_t _type, const std::string &_name, const glm::vec3 &_position, const std::string &_material_name);
    
    // 
    void on_keydown_event(const event_t &_e);

    // windows
    void draw_transform_window();
    void draw_material_window();
    void draw_color_picker_window();
    void draw_hierarchy_window();
    void draw_texture_select_window();
    void draw_create_primitive_window();
    
    // 
    void toggle_create_menu(const glm::vec2 &_pos);
    void hide_create_menu();

    // 
    void open_texture_select();
    void assign_texture_to_selected(const std::string &_name);
    // void set_texture_select_preview(const std::string &_name);

    // 
    entity_handle_t create_primitive(primitive_type_t _type);
    entity_handle_t pick_entity(const glm::vec2 &_screen_pos);
    ui_transform_axis_t pick_ui_transform_axis(const glm::vec2 &_screen_pos);
    void begin_ui_transform_drag();
    void update_ui_transform_drag(glm::vec2 &_screen_pos);

    //
    void open_color_picker(const glm::vec2 &_anchor_pos);
    void close_color_picker(bool _apply);
    void update_color_picker_from_hsv();  // HSV changed, update RGB float fields and material
    void update_color_picker_from_rgb();  // float fields changed, update HSV and material

    // undo stack
    void push_transform_command(entity_handle_t _entity_handle, 
                                const glm::vec3 &_prev_pos,
                                const glm::vec3 &_prev_rot,
                                const glm::vec3 &_prev_scale,
                                const glm::vec3 &_next_pos,
                                const glm::vec3 &_next_rot,
                                const glm::vec3 &_next_scale);
    void push_material_command(entity_handle_t _entity_handle, 
                               const material_pbr_payload_t &_prev, 
                               const material_pbr_payload_t &_next);
    void push_texture_command(entity_handle_t _entity_handle, texture_handle_t _prev, texture_handle_t _next);
    void undo();
    
    // window handle accessors
    void set_transform_window_handle(window_handle_t _handle) { m_transform_window_handle = _handle; }
    void set_material_window_handle(window_handle_t _handle) { m_material_window_handle = _handle; }
    void set_color_picker_window_handle(window_handle_t _handle) { m_color_picker_window_handle = _handle; }
    void set_hierarchy_window_handle(window_handle_t _handle) { m_hierarchy_window_handle = _handle; }
    void set_texture_select_window_handle(window_handle_t _handle) { m_texture_select_window_handle = _handle; }
    void set_create_window_handle(window_handle_t _handle) { m_create_window_handle = _handle; }

    window_handle_t get_transform_window_handle() { return m_transform_window_handle; }
    window_handle_t get_material_window_handle() { return m_material_window_handle; }
    window_handle_t  get_color_picker_window_handle() { return m_color_picker_window_handle; }
    window_handle_t get_hierarchy_window_handle() { return m_hierarchy_window_handle; }
    window_handle_t get_texture_select_window_handle() { return m_texture_select_window_handle; }
    window_handle_t get_create_window_handle() { return m_create_window_handle; }

    // transform ui accessors
    void set_hovered_ui_transform_axis(ui_transform_axis_t _axis) { m_hovered_ui_transform_axis = _axis; }
    void set_grabbed_ui_transform_axis(ui_transform_axis_t _axis) { m_grabbed_ui_transform_axis = _axis; }
    void set_ui_transform_mode(ui_transform_mode_t _mode) { m_ui_transform_mode = _mode; }

    ui_transform_axis_t get_hovered_ui_transform_axis() { return m_hovered_ui_transform_axis; }
    ui_transform_axis_t get_grabbed_ui_transform_axis() { return m_grabbed_ui_transform_axis; }
    ui_transform_mode_t get_ui_transform_mode() { return m_ui_transform_mode; }
    
private:
    window_handle_t m_transform_window_handle      = { 0 };
    window_handle_t m_material_window_handle       = { 0 };
    window_handle_t m_color_picker_window_handle   = { 0 };
    window_handle_t m_hierarchy_window_handle      = { 0 };
    window_handle_t m_texture_select_window_handle = { 0 };
    window_handle_t m_create_window_handle         = { 0 };
    
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

public:
    glm::vec3 m_drag_prev_pos;
    glm::vec3 m_drag_prev_rot;
    glm::vec3 m_drag_prev_scale;

    // color picker
public:
    glm::vec3 m_color_picker_hsv = { 0.0f, 1.0f, 1.0f };
    glm::vec3 m_color_picker_rgb = { 1.0f, 0.0f, 0.0f };
private:
    glm::vec4 m_color_picker_prev_color = {};   // for 'Cancel'

    // undo stack variables
    glm::vec3 m_prev_transform_position;
    glm::vec3 m_prev_transform_rotation;
    glm::vec3 m_prev_transform_scale;
    
    int m_texture_selected_index = -1;
    texture_handle_t m_texture_preview_handle = { 0 };

    
    // undo
    undo_stack_t m_undo_stack;
    
};



#endif // __EDITOR_H

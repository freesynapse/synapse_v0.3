#ifndef __EDITOR_H
#define __EDITOR_H

#include "renderer/mesh/mesh_generator.h"
#include "renderer/entity/entity_types.h"
#include "renderer/UI/window/window_types.h"
#include "event/event.h"


// 
class editor_t
{
public:
    void init();

    void on_keydown_event(const event_t &_e);
        
    void toggle_create_menu(const glm::vec2 &_pos);
    void hide_create_menu();

    void open_texture_select();
    void assign_texture_to_selected(const std::string &_name);
    void set_texture_select_preview(const std::string &_name);
    
    entity_handle_t create_primitive(primitive_type_t _type);
    entity_handle_t pick_entity(const glm::vec2 &_screen_pos);

    // 
    void set_create_window_handle(window_handle_t _handle) { m_create_window_handle = _handle; }
    void set_tex_picker_window_handle(window_handle_t _handle) { m_tex_picker_window_handle = _handle; }

    window_handle_t get_create_window_handle() { return m_create_window_handle; }
    window_handle_t get_tex_picker_window_handle() { return m_tex_picker_window_handle; }

private:
    window_handle_t m_create_window_handle = { 0 };
    window_handle_t m_tex_picker_window_handle = { 0 };
};



#endif // __EDITOR_H


#include "dev_tools.h"

#include "event/input_manager.h"
#include "utils/file_io_handler.h"

#include "c_api.h"


// 
void dev_tools_t::handle_input()
{
    if (input.was_key_pressed(m_fullscreen_toggle_key)) {
        event_t e;
        e.type = event_type_t::WINDOW_TOGGLE_FULLSCREEN;
        events.dispatch_event(e);
    }

    if (input.was_key_pressed(SYN_KEY_TAB)) {
        cam.toggle_mode();
    }

    if (input.was_key_pressed(SYN_KEY_F1)) {
        window_t *hw = window_manager.get_window(window_manager.get_help_window_handle());
        if (hw) {
            hw->set_visible(!hw->is_visible());            
        }
    }
    
    if (input.was_key_pressed(SYN_KEY_F2)) {
        renderer.toggle_wireframe();
    }

    if (input.was_key_pressed(SYN_KEY_F3)) {
        renderer.toggle_perf_overlay();
    }

    if (input.was_key_pressed(SYN_KEY_F5)) {
        renderer.toggle_normals();
    }

    if (input.was_key_pressed(SYN_KEY_F6)) {
        renderer.toggle_tangents();
    }

    if (input.was_key_pressed(SYN_KEY_F7)) {
        renderer.toggle_bounding_boxes();
    }

    if (input.was_key_pressed(SYN_KEY_F8)) {
        shader_lib.reload_shaders();
    }

    if (input.was_key_pressed(SYN_KEY_F10)) {
        renderer.toggle_grid();
    }
    
}

// 
void dev_tools_t::load_help_file(const std::string &_file_path)
{
    file_io_handler.read_file_to_lines(_file_path, m_help_content);
    
}

#include <memory.h>

#include "c_api.h"
#include "dev_tools.h"

#include "glfw_window.h"
#include "event/input_manager.h"
#include "event/event_handler.h"
#include "renderer/renderer.h"
#include "renderer/shader/shader_library.h"
#include "renderer/mesh/mesh_library.h"
#include "utils/log.h"
#include "utils/time_step.h"
#include "utils/scope_timer.h"


//---------------------------------------------------------------------------------------
// globals

// engine core
glfw_window_t               root_window;
gl_api_t                    api;
input_handler_t             input;
events_t                    events;
renderer_t                  renderer;
renderer_2d_t               renderer_2d;
time_step_t                 time_step;
file_io_handler_t           file_io_handler;
shader_library_t            shader_lib;
texture_library_t           tex_lib;
material_library_t          mat_lib;
mesh_library_t              mesh_lib;
cubemap_library_t           cubemap_lib;
entity_library_t            entity_lib;
asset_manager_t             assets;
window_manager_t            window_manager;
random_t                    rng;
mesh_generator_t            mesh_generator;
editor_t                    editor;
dev_tools_t                 dev_tools;

// rendering
font_t                      font;
camera_controller_t         cam;
perspective_camera_t        perspective_camera;
orbit_camera_t              orbit_camera;
orthographic_camera_t       orthographic_camera;

// editing
entity_handle_t             selected_entity_handle = { 0 };


//---------------------------------------------------------------------------------------
// high-level control
//
void syn_init(const char *_name, int _width, int _height, int _mode)
{
    // open log file
    syn_open_log();

    // initalize event handler
    events.init();

    // create GLFW window
    if (root_window.init(_name, _width, _height) == -1) {
		SYN_ERROR("GLFW window initialization failed. Terminating.");
		glfwTerminate();
	} else {
	    SYN_INFO("GLFW window initialized.\n");
	}

    // core systems (cont.)
    input.init();
    rng.init();
    api.init();
    api.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
    renderer.init();
    renderer_2d.init();
    tex_lib.init();
    mat_lib.init();
    cubemap_lib.init();
    font.init("../assets/font/JetBrainsMono-Regular.ttf", 16);
    font.set_color(glm::vec4(1.0f));

    window_manager.init();
    editor.init();

    // select main rendering mode
    switch (_mode) {
        case SYN_MODE_2D: syn_mode_2d(); SYN_INFO("SYN_MODE_2D enabled.\n"); break;
        case SYN_MODE_3D: syn_mode_3d(); SYN_INFO("SYN_MODE_3D enabled.\n"); break;
        default: SYN_FATAL_ERROR("unknown rendering mode selected.\n");
    }

}

//
void syn_mode_2d()
{
    glm::ivec2 dims = root_window.get_window_dims();
    orthographic_camera = orthographic_camera_t((float)dims.x / (float)dims.y);
}

//
void syn_mode_3d()
{
    glm::ivec2 dims = root_window.get_window_dims();
    orbit_camera.init(60.0f, dims.x, dims.y, 0.1f, 1000.0f);
    // sensible defaults
    orbit_camera.m_orbit_speed = 0.5f;
    orbit_camera.m_x_angle = 315.0f;
    orbit_camera.m_y_angle = 60.0f;
    orbit_camera.m_distance = 14.0f;
    
    perspective_camera.init(60.0f, dims.x, dims.y, 0.1f, 1000.0f);
    cam.init(&orbit_camera, &perspective_camera);
}

//
void syn_shutdown()
{
    font.destroy();

    shader_lib.shutdown();
    mesh_lib.shutdown();
    cubemap_lib.shutdown();
    window_manager.shutdown();
    
    renderer.shutdown();
    renderer_2d.shutdown();

    syn_close_log();
    root_window.destroy();

}


//---------------------------------------------------------------------------------------
// ui creation and ui asset loader
//
void syn_load_assets(const char *_asset_file)
{
    scope_timer_t t;
    assets.load_manifest(_asset_file);

    // bake hdr maps if skybox is loaded
    if (renderer.m_skybox.is_active) {
        SYN_INFO("baking HDR irradiance and specular maps.\n");
        renderer.bake_ibl();
    }

    SYN_INFO("assets loaded in %.2f ms.\n", t.get_dt_ms());

    // reset performance statistics
    renderer.m_perf_stats.frame_time_idx = 0;
    memset(renderer.m_perf_stats.frame_times, 0, sizeof(renderer.m_perf_stats.frame_times));

}

// 
void syn_load_ui_layout(const char *_filepath)
{
    FILE *fp = fopen(_filepath, "r");
    if (!fp) {
        SYN_WARNING("could not open layout file '%s'.\n", _filepath);
        return;
    }

    float sw = root_window.get_fwidth();
    float sh = root_window.get_fheight();

    char line[256];
    char type[64] = "";
    char name[128] = "";

    glm::vec2 position = { 0.0f, 0.0f };
    glm::vec2 size     = { 0.5f, 0.5f };

    auto flush_window = [&]() {
        if (strlen(type) == 0) return;
        glm::vec2 abs_pos  = { position.x * sw, position.y * sh };
        glm::vec2 abs_size = { size.x * sw,     size.y * sh     };
        if      (strcmp(type, "viewport")       == 0) syn_create_viewport_window(name, abs_pos, abs_size);
        else if (strcmp(type, "log")            == 0) syn_create_log_window(name, abs_pos, abs_size);
        else if (strcmp(type, "hierarchy")      == 0) syn_create_hierarchy_window(name, abs_pos, abs_size);
        else if (strcmp(type, "transform")      == 0) syn_create_transform_window(name, abs_pos, abs_size);
        else if (strcmp(type, "material")       == 0) syn_create_material_window(name, abs_pos, abs_size);
        else if (strcmp(type, "creator")        == 0) syn_create_primitive_window(name, abs_pos, abs_size);
        else if (strcmp(type, "texture_select") == 0) syn_create_texture_select_window(name, abs_pos, abs_size);
        else if (strcmp(type, "help")           == 0) syn_create_help_window(name, abs_pos, abs_size);
        else if (strcmp(type, "window") == 0) {
            window_t win;
            win.name = name;
            win.position = abs_pos;
            win.size = abs_size;
            window_manager.add_window(win);
        }
        
        memset(type, 0, sizeof(type));
        memset(name, 0, sizeof(name));
        position = { 0.0f, 0.0f };
        size     = { 0.5f, 0.5f };
    };

    while (fgets(line, sizeof(line), fp)) {
        // strip newline
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (line[0] == '#') {
            // commit previous block
            flush_window();
            sscanf(line + 1, "%s", type);
        }
        else if (strncmp(line, "name", 4) == 0)     sscanf(line + 4, " %[^\n]", name);
        else if (strncmp(line, "position", 8) == 0) sscanf(line + 8, " %f %f", &position.x, &position.y);
        else if (strncmp(line, "size", 4) == 0)     sscanf(line + 4, " %f %f", &size.x, &size.y);
    }

    // commit last block
    flush_window();
    fclose(fp);

    // load other windows, not in the layout file
    syn_create_color_picker_window("Color Picker", { 400.0f, 200.0f }, { 320.0f, 280.0f });
    
    SYN_INFO("layout loaded from '%s'.\n", _filepath);
    
}

// 
void syn_save_ui_layout(const char *_filepath)
{
    FILE *fp = fopen(_filepath, "w");
    if (!fp) {
        SYN_WARNING("could not open layout file '%s' for writing.\n", _filepath);
        return;
    }

    float sw = root_window.get_fwidth();
    float sh = root_window.get_fheight();

    for (uint32_t i = 0; i < SYN_MAX_WINDOW_COUNT; i++) {
        window_t *win = window_manager.get_window({ i + 1 });
        if (!win || !win->is_active()) continue;
        if (win->is_tab_child()) continue;
     
        const char *type = "window";
        if      (win->handle().id == window_manager.get_viewport_window_handle().id)  type = "viewport";
        else if (win->handle().id == window_manager.get_log_window_handle().id)       type = "log";
        else if (win->handle().id == window_manager.get_hierarchy_window_handle().id) type = "hierarchy";
        else if (win->handle().id == window_manager.get_transform_window_handle().id) type = "transform";
        else if (win->handle().id == window_manager.get_material_window_handle().id)  type = "material";
        else if (win->handle().id == editor.get_create_window_handle().id)            type = "create";
        else if (win->handle().id == editor.get_texture_select_window_handle().id)    type = "texture_select";
        else if (win->handle().id == window_manager.get_help_window_handle().id)      type = "help";
        
        fprintf(fp, "#%s\n", type);
        fprintf(fp, "name        %s\n", win->name.c_str());
        fprintf(fp, "position    %.4f %.4f\n", win->position.x / sw, win->position.y / sh);
        fprintf(fp, "size        %.4f %.4f\n", win->size.x / sw,     win->size.y / sh);
        fprintf(fp, "\n");
    }

    fclose(fp);
    SYN_INFO("layout saved to '%s'.\n", _filepath);
    
}

// 
void syn_create_viewport_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name = _name;
    win.position = _pos;
    win.size = _size;
    win.set_resizable(false);

    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_viewport_window(handle);    
    window_t *w = window_manager.get_window(handle);
    w->create_framebuffer();
    
}

// 
void syn_create_log_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;

    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_log_window_handle(handle);

}

// 
void syn_create_hierarchy_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;

    window_handle_t handle = window_manager.add_window(win);
    editor.set_hierarchy_window_handle(handle);
    
}

// 
void syn_create_transform_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;
    window_handle_t handle = window_manager.add_window(win);
    editor.set_transform_window_handle(handle);

}

// 
void syn_create_material_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;
    window_handle_t handle = window_manager.add_window(win);
    editor.set_material_window_handle(handle);
    
}

// 
void syn_create_primitive_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    float btn_h = 24.0f;  // match im_default_row_height
    float min_h = 4.0f + (int)primitive_type_t::COUNT * (btn_h + 4.0f);

    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = { _size.x, glm::max(_size.y, min_h) };
    win.set_resizable(false);
    window_handle_t handle = window_manager.add_window(win);
    editor.set_create_window_handle(handle);
    window_t *pw = window_manager.get_window(handle);
    pw->set_visible(false);

}

// 
void syn_create_texture_select_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;

    window_handle_t handle = window_manager.add_window(win);
    editor.set_texture_select_window_handle(handle);

    window_t *pw = window_manager.get_window(handle);
    pw->set_visible(false);
    pw->set_resizable(true);

}

// 
void syn_create_help_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;
    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_help_window_handle(handle);
    window_t *hw = window_manager.get_window(handle);
    hw->set_visible(false);
    dev_tools.load_help_file("../assets/docs/help.txt");

}

// 
void syn_create_color_picker_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name     = _name;
    win.position = _pos;
    win.size     = _size;
    window_handle_t handle = window_manager.add_window(win);
    editor.set_color_picker_window_handle(handle);

    window_t *pw = window_manager.get_window(handle);
    pw->set_visible(false);
    pw->set_resizable(false);

}


//---------------------------------------------------------------------------------------
// rendering loop functions
//
bool __was_prerender_called = false; 
void syn_prerender()
{
    if (__was_prerender_called) return;

    root_window.pre_render();
    renderer.reset_perf_counters();

    events.process_events();
    
    __was_prerender_called = true;
    
}

// 
void syn_render_begin_3d()
{
    syn_prerender();
    
    //
    api.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
    renderer.bind_scene_fbuffer();
    
    // perspective_camera.update(time_step.dt);
    cam.update(time_step.dt);

}

//
void syn_render_end_3d()
{
    // dev tools
    if (renderer.debug.show_normals || renderer.debug.show_tangents) {
        entity_t *e = entity_lib.get_entity(selected_entity_handle);
        if (e){
            renderer.render_debug_normals(e->mesh_handle, e->transform);
        }
    }
    if (renderer.debug.show_bounding_boxes) { renderer.render_debug_bounding_box_entities(); }
    if (renderer.debug.show_grid) { renderer.render_debug_grid(); }

    // always render AABB around selected entity
    if (selected_entity_handle.is_valid()) {
        entity_t *e = entity_lib.get_entity(selected_entity_handle);
        if (e) {
            bool prev = renderer.debug.show_bounding_boxes;
            renderer.debug.show_bounding_boxes = true;
            renderer.render_debug_bounding_box_entities(e);
            renderer.debug.show_bounding_boxes = prev;

            renderer.render_ui_transform(e->t_position);
        }
    }
    
    //
    renderer.render_orientation_obj();

    // draw_perf_overlay does NOT contain font.start_/.end_render_block()
    renderer.record_frame_time(time_step.dt * 1000.0f);

    // render all scene text
    font.end_render_block(false);
    
    // render scene in viewport window
    window_t *viewport = window_manager.get_viewport_window();
    if (viewport && viewport->has_frambuffer()) {
        api.fbo_handler.unbind();
    }

    // 
    syn_render_end();

}

//
void syn_render_end()
{
    // everything is drawn, render the screen NDC quad
    api.set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
    api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    // ui rendering
    api.clear_depth_buffer();
    window_manager.draw_windows();

    // 
    renderer.draw_perf_stats();
    renderer.draw_notifications();
    font.end_render_block(false);

    // 
    dev_tools.handle_input();
}

// 
void syn_frame_end()
{
    root_window.post_render();
    time_step.update();
    time_step.calculate_fps();

    __was_prerender_called = false;
}


//---------------------------------------------------------------------------------------
// immediate mode ui
// 
void syn_im_begin()
{
    renderer_2d.batch.begin_batch();
    
}

// 
void syn_im_end()
{
    renderer_2d.batch.end_batch();
    font.end_render_block(true);
}

void syn_begin_window(window_handle_t _handle)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !w->is_visible()) return;
    w->im_begin();
}

// 
void syn_end_window(window_handle_t _handle)
{
    
}

// 
void syn_begin_row(window_handle_t _handle, const std::vector<float> &_ratios)
{
    window_t *w = window_manager.get_window(_handle);
    if (w) w->im_begin_row(_ratios);
}

// 
void syn_end_row(window_handle_t _handle)
{
    window_t *w = window_manager.get_window(_handle);
    if (w) w->im_end_row();
    
}

// immediate mode widgets
// 
void syn_label(window_handle_t _handle, const char *_text)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);
    
    float font_depth = w->depth + window_manager.ddepth_layer_text;

    font.set_depth(font_depth);
    font.set_color(glm::vec4(1.0f));
    font.render_text_clipped(wp.p.x + 4.0f, wp.p.y + (wp.s.y + font.get_font_glyph_height()) * 0.5f, wp.s.x, _text);

    im_update_cursor_y(w, &wp);
}

// 
bool syn_button(window_handle_t _handle, const char *_text)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w) return false;

    uint32_t id = im_widget_hash(_text, w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return false;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    state->is_hovered = w->im_is_hovered(wp.p, wp.s);
    glm::vec4 col = state->is_hovered ? glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    renderer_2d.batch.add_quad(wp.p, wp.s, col, w->depth + window_manager.ddepth_layer_widget);

    ui_render_vertex_t outline[] = {
        ui_render_vertex_t({ wp.p.x,          wp.p.y          }, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x, wp.p.y          }, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x, wp.p.y + wp.s.y }, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,          wp.p.y + wp.s.y }, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,          wp.p.y          }, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), w->depth + 0.02f),
    };
    renderer_2d.batch.add_line_strip(outline, 5);

    if (_text) {
        float tx = wp.p.x + (wp.s.x - font.get_string_width(_text)) * 0.5f;
        float ty = wp.p.y + (wp.s.y + font.get_font_glyph_height()) * 0.5f;
        font.set_depth(w->depth + window_manager.ddepth_layer_text);
        font.set_color(glm::vec4(1.0f));
        font.render_text(tx, ty, _text);
    }

    // return true if clicked
    im_update_cursor_y(w, &wp);
    return state->is_hovered && input.is_button_pressed(SYN_MOUSE_BUTTON_1);
    
}

// 
void syn_float_field(window_handle_t _handle,
                     const char *_label,
                     float *_value,
                     float _min,
                     float _max)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !_value) return;

    uint32_t id = im_widget_hash(_label, w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    if (!state->binding) state->binding = _value;
    state->min = _min;
    state->max = _max;
    
    widget_params_t wp;
    im_set_widget_params(w, &wp);

    state->is_hovered = w->im_is_hovered(wp.p, wp.s);

    // sync value from binding when not editing
    if (!state->editing) state->value = *_value;
    glm::vec4 col = state->is_hovered ? glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) : glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
    renderer_2d.batch.add_quad(wp.p, wp.s, col, w->depth + window_manager.ddepth_layer_widget);

    glm::vec4 ol_c = state->editing ? glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) : glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    ui_render_vertex_t outline[] = {
        ui_render_vertex_t({ wp.p.x,          wp.p.y          }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x, wp.p.y          }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x, wp.p.y + wp.s.y }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,          wp.p.y + wp.s.y }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,          wp.p.y          }, ol_c, w->depth + 0.02f),
    };
    renderer_2d.batch.add_line_strip(outline, 5);

    float font_depth = w->depth + window_manager.ddepth_layer_text;

    font.set_depth(font_depth);
    float text_y = wp.p.y + (wp.s.y + font.get_font_glyph_height()) * 0.5f;
    font.set_color(glm::vec4(1.0f));

    // position label left of field
    if (_label) {
        float label_x = wp.p.x + 6.0f;
        font.render_text(label_x, text_y, _label);
    }

    // value display
    char display[SYN_IM_BUFFER_LEN] = {};
    if (state->editing) snprintf(display, sizeof(display), "%s", state->buf);
    else                snprintf(display, sizeof(display), "%.3f", state->value);

    float val_w = font.get_string_width("%s", display);
    float val_x = wp.p.x + wp.s.x - val_w - 6.0f;
    font.render_text(val_x, text_y, display);

    // cursor
    if (state->editing) {
        char before[SYN_IM_BUFFER_LEN] = {};
        strncpy(before, state->buf, state->cursor);
        float cur_x = val_x + font.get_string_width(before);
        ui_render_vertex_t cur[] = {
            ui_render_vertex_t({ cur_x, wp.p.y + 3.0f          }, glm::vec4(1.0f), w->depth + 0.02f),
            ui_render_vertex_t({ cur_x, wp.p.y + wp.s.y - 3.0f }, glm::vec4(1.0f), w->depth + 0.02f),
        };
        renderer_2d.batch.add_line_strip(cur, 2);
    }

    // click to start editing
    if (state->is_hovered && w->im_click_pending) {
        // cancel editing of all other widgets in window
        for (uint32_t i = 0; i < w->im_widget_state_count; i++) {
            w->im_widget_states[i].editing = false;
        }

        // enable this field for editing
        state->editing = true;
        w->im_active_state_id = state->id;
        snprintf(state->buf, SYN_IM_BUFFER_LEN, "%.3f", state->value);
        state->cursor = (int)strlen(state->buf);
        w->im_click_pending = false;
    }

    // consume scroll
    if (state->is_hovered && !state->editing && w->im_scroll_delta != 0.0f) {
        if (!state->is_scrolling) {
            state->pre_scroll_value = state->value;
            state->is_scrolling = true;
        }
        float speed = 1.0f;
        if (input.is_key_down(SYN_KEY_LEFT_SHIFT))  speed = 10.0f;
        if (input.is_key_down(SYN_KEY_LEFT_ALT))    speed = 0.1f;
        if (input.is_key_down(SYN_KEY_LEFT_CTRL))   speed = 0.01f;
        float new_val = glm::clamp(state->value - w->im_scroll_delta * speed, state->min, state->max);
        state->value = new_val;
        if (state->binding) *state->binding = new_val;
        w->im_scroll_delta = 0.0f;
        
    }

    else if (state->is_scrolling && w->im_scroll_delta == 0.0f) {
        state->is_scrolling = false;
        state->is_dirty = true;
    }
    
    im_update_cursor_y(w, &wp);
    
}

// 
void syn_color_picker_sv(window_handle_t _handle, float *_hue, float *_saturation, float *_value)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !_hue || !_saturation || !_value) return;

    uint32_t id = im_widget_hash("__color_picker_sv", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    // override height: remaining space minus rgb row + button row + padding
    float rows_below = 24.0f * 2.0f + wp.pad * 3.0f;
    wp.s.y = w->get_content_size().y - w->im_cursor_y - rows_below;
    wp.p.y = wp.content_pos.y + w->im_cursor_y;

    state->is_hovered = w->im_is_hovered(wp.p, wp.s);

    // draw SV quad (shader)
    shader_t *shader = shader_lib.get_shader(renderer.m_ui_color_picker_shader_handle);
    if (shader) {
        shader->enable();
        shader->set_matrix_4fv("u_projection", renderer.m_ui_projection);
        shader->set_uniform_2fv("u_position", wp.p);
        shader->set_uniform_2fv("u_size", wp.s);
        shader->set_uniform_1f("u_depth", w->depth + window_manager.ddepth_layer_widget);
        shader->set_uniform_1i("u_mode", 0);
        shader->set_uniform_1f("u_hue", *_hue);
        renderer.m_ui_tex_quad_vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        renderer.m_ui_tex_quad_vao.unbind();
        shader->disable();
    }

    // crosshair
    float cx = wp.p.x + *_saturation * wp.s.x;
    float cy = wp.p.y + (1.0f - *_value) * wp.s.y;
    float arm = 6.0f, thickness = 1.5f;
    glm::vec4 cross_col = { 1.0f, 1.0f, 1.0f, 0.75f };
    renderer_2d.batch.add_quad({ cx - arm, cy - thickness * 0.5f }, { arm * 2.0f, thickness }, cross_col, w->depth + 0.01f);
    renderer_2d.batch.add_quad({ cx - thickness * 0.5f, cy - arm }, { thickness, arm * 2.0f }, cross_col, w->depth + 0.01f);
    
    // click/drag
    if (state->is_hovered && w->im_click_pending) {
        w->im_dragging_state_id = state->id;
        w->im_drag_type         = im_drag_type_t::COLOR_PICKER_SV;
        w->im_drag_widget_pos   = wp.p;
        w->im_drag_widget_size  = wp.s;
        float s = glm::clamp((w->im_click_pos.x - wp.p.x) / wp.s.x, 0.0f, 1.0f);
        float v = glm::clamp(1.0f - (w->im_click_pos.y - wp.p.y) / wp.s.y, 0.0f, 1.0f);
        *_saturation = s;
        *_value = v;
        editor.update_color_picker_from_hsv();
        w->im_click_pending = false;
    }

    im_update_cursor_y(w, &wp);
    
}

// 
void syn_color_picker_hue(window_handle_t _handle, float *_hue)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !_hue) return;

    uint32_t id = im_widget_hash("__color_picker_hue", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    // override height: remaining space minus rgb row + button row + padding
    float rows_below = 24.0f * 2.0f + wp.pad * 3.0f;
    wp.s.y = w->get_content_size().y - w->im_cursor_y - rows_below;
    wp.p.y = wp.content_pos.y + w->im_cursor_y;

    state->is_hovered = w->im_is_hovered(wp.p, wp.s);

    // draw hue bar (shader)
    shader_t *shader = shader_lib.get_shader(renderer.m_ui_color_picker_shader_handle);
    if (shader) {
        shader->enable();
        shader->set_matrix_4fv("u_projection", renderer.m_ui_projection);
        shader->set_uniform_2fv("u_position", wp.p);
        shader->set_uniform_2fv("u_size", wp.s);
        shader->set_uniform_1f("u_depth", w->depth + window_manager.ddepth_layer_widget);
        shader->set_uniform_1i("u_mode", 1);
        shader->set_uniform_1f("u_hue", 0.0f);
        renderer.m_ui_tex_quad_vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        renderer.m_ui_tex_quad_vao.unbind();
        shader->disable();
    }

    // indicator bar
    float hy = wp.p.y + (1.0f - *_hue) * wp.s.y;
    renderer_2d.batch.add_quad({ wp.p.x, hy - 1.5f }, { wp.s.x, 3.0f }, { 1.0f, 1.0f, 1.0f, 0.75f }, w->depth + 0.01f);

    // click/drag
    if (state->is_hovered && w->im_click_pending) {
        w->im_dragging_state_id = state->id;
        w->im_drag_type         = im_drag_type_t::COLOR_PICKER_HUE;
        w->im_drag_widget_pos   = wp.p;
        w->im_drag_widget_size  = wp.s;
        float h = glm::clamp(1.0f - (w->im_click_pos.y - wp.p.y) / wp.s.y, 0.0f, 1.0f);
        *_hue = h;
        editor.update_color_picker_from_hsv();
        w->im_click_pending = false;
    }

    im_update_cursor_y(w, &wp);
    
}

// 
void syn_color_swatch(window_handle_t _handle, float *_r, float *_g, float *_b)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !_r || !_g || !_b) return;

    uint32_t id = im_widget_hash("__color_swatch", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    state->is_hovered = w->im_is_hovered(wp.p, wp.s);

    glm::vec4 col = { *_r, *_g, *_b, 1.0f };
    renderer_2d.batch.add_quad(wp.p, wp.s, col, w->depth + 0.01f);

    glm::vec4 ol_c = state->is_hovered ? glm::vec4(0.8f, 0.8f, 0.8f, 1.0f) : glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    
    ui_render_vertex_t outline[] = {
        ui_render_vertex_t({ wp.p.x,           wp.p.y           }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x,  wp.p.y           }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x + wp.s.x,  wp.p.y + wp.s.y  }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,           wp.p.y + wp.s.y  }, ol_c, w->depth + 0.02f),
        ui_render_vertex_t({ wp.p.x,           wp.p.y           }, ol_c, w->depth + 0.02f),
    };
    renderer_2d.batch.add_line_strip(outline, 5);

    // open color picker on click
    if (state->is_hovered && w->im_click_pending) {
        editor.open_color_picker(w->im_click_pos);
        w->im_click_pending = false;
    }

    im_update_cursor_y(w, &wp);
    
}

bool syn_list(window_handle_t _handle,
              const char **_items,
              uint32_t _count,
              int *_selected_index,
              int *_hovered_index,
              float _max_height)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w || !_items || !_selected_index) return false;

    uint32_t id = im_widget_hash("__list", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return false;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    // fill remaining content
    wp.s.y = (_max_height > 0.0f) ? _max_height : w->get_content_size().y - w->im_cursor_y;
    wp.p.y = wp.content_pos.y + w->im_cursor_y;

    float row_h = font.get_font_glyph_height() + 6.0f;
    uint32_t max_rows = (uint32_t)(wp.s.y / row_h);
    uint32_t start = (uint32_t)state->scroll_offset;

    font.set_depth(w->depth + window_manager.ddepth_layer_text);
    font.set_color(glm::vec4(1.0f));

    bool selection_changed = false;
    uint32_t drawn = 0;
    
    if (_hovered_index) *_hovered_index = -1;

    for (uint32_t i = start; i < _count && drawn < max_rows; i++) {
        float row_y = wp.p.y + drawn * row_h;
        glm::vec2 row_p = { wp.p.x, row_y };
        glm::vec2 row_s = { wp.s.x, row_h };

        bool is_selected = ((int)i == *_selected_index);
        if (is_selected) {
            renderer_2d.batch.add_quad(row_p, row_s,
                glm::vec4(0.5f, 0.3f, 0.1f, 1.0f),
                w->depth + window_manager.ddepth_layer_widget);
        } else if (w->im_is_hovered(row_p, row_s)) {
            renderer_2d.batch.add_quad(row_p, row_s,
                glm::vec4(0.4f, 0.4f, 0.4f, 1.0f),
                w->depth + window_manager.ddepth_layer_widget);
        }

        font.render_text_clipped(wp.p.x + 6.0f, row_y + row_h - font.get_font_glyph_height() * 0.5f, wp.s.x - 8.0f, "%s", _items[i]);

        if (w->im_click_pending && w->im_is_hovered(row_p, row_s)) {
            *_selected_index = (int)i;
            selection_changed = true;
            w->im_click_pending = false;
        }

        // 
        if (w->im_is_hovered(row_p, row_s)) {
            renderer_2d.batch.add_quad(
                row_p, row_s, 
                glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), 
                w->depth + window_manager.ddepth_layer_widget
            );
            if (_hovered_index) *_hovered_index = (int)i;
        }
        
        drawn++;
    }

    // scroll
    if (w->im_is_hovered(wp.p, wp.s) && w->im_scroll_delta != 0.0f) {
        float max_scroll = (float)(_count > max_rows ? _count - max_rows : 0);
        state->scroll_offset = glm::clamp(state->scroll_offset - w->im_scroll_delta, 0.0f, max_scroll);
        w->im_scroll_delta = 0.0f;
    }

    im_update_cursor_y(w, &wp);

    return selection_changed;
    
}

// 
void syn_tex_quad(window_handle_t _handle, texture_handle_t _tex_handle, const glm::vec2 &_size, const glm::vec2 &_pos)
{
    window_t *w = window_manager.get_window(_handle);
    if (!w) return;

    texture_internal_t *tex = tex_lib.get_texture(_tex_handle);
    if (!tex) return;
    
    uint32_t id = im_widget_hash("__tex_quad", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);
    if (_pos.x >= 0.0f) {
        wp.p = _pos;
        wp.s = _size;
    } else {
        wp.s = _size;
    }

    shader_t *shader = shader_lib.get_shader(renderer.m_ui_tex_quad_shader_handle);
    if (!shader) return;

    shader->enable();
    shader->set_matrix_4fv("u_projection", renderer.m_ui_projection);
    shader->set_uniform_2fv("u_position", wp.p);
    shader->set_uniform_2fv("u_size", wp.s);
    shader->set_uniform_1f("u_depth", w->depth + window_manager.ddepth_layer_widget);
    glBindTextureUnit(0, tex->opengl_id);
    renderer.m_ui_tex_quad_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    renderer.m_ui_tex_quad_vao.unbind();
    shader->disable();

    im_update_cursor_y(w, &wp);    
    
}

// 
void syn_log_window()
{
    window_handle_t handle = window_manager.get_log_window_handle();
    
    syn_begin_window(handle);
    
    window_t *w = window_manager.get_window(handle);
    if (!w) return;

    uint32_t id = im_widget_hash("__log", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) return;

    widget_params_t wp;
    im_set_widget_params(w, &wp);

    // fill remaining content
    wp.s.y = w->get_content_size().y - w->im_cursor_y;
    wp.p.y = wp.content_pos.y + w->im_cursor_y;

    float line_h = font.get_font_height() + 2.0f;
    uint32_t max_lines = (uint32_t)(wp.s.y / line_h);
    uint32_t count = syn_log_buffer.count;

    // auto-scroll: offset 0 = bottom, > 0 = scroll up
    uint32_t start = 0;
    if (count > max_lines) {
        float max_scroll = (float)(count - max_lines);
        float offset = glm::clamp(state->scroll_offset, 0.0f, max_scroll);
        start = (uint32_t)(max_scroll - offset);
    }

    uint32_t display_count = (count > start) ? std::min(count - start, max_lines) : 0;
    font.set_depth(w->depth + window_manager.ddepth_layer_text);
    
    for (uint32_t i = 0; i < display_count; i++) {
        const log_entry_t &e = syn_log_buffer.get(start + i);
        glm::vec4 color;
        switch (e.level) {
            case log_level_t::WARNING: color = { 1.0f, 0.8f, 0.0f, 1.0f }; break;
            case log_level_t::ERROR:   color = { 1.0f, 0.3f, 0.3f, 1.0f }; break;
            case log_level_t::DEBUG:   color = { 0.4f, 1.0f, 0.4f, 1.0f }; break;
            default:                   color = { 1.0f, 1.0f, 1.0f, 1.0f }; break;
        }
        font.set_color(color);
        float text_w = wp.s.x - 14.0f;
        font.render_text_clipped(wp.p.x + 4.0f, wp.p.y + i * line_h + line_h, text_w, e.msg);
    }

    // scroll
    if (w->im_is_hovered(wp.p, wp.s) && w->im_scroll_delta != 0.0f) {
        float max_scroll = (float)(count > max_lines ? count - max_lines : 0);
        state->scroll_offset = glm::clamp(state->scroll_offset + w->im_scroll_delta, 0.0f, max_scroll);
        w->im_scroll_delta = 0.0f;
    }

    // scroll-bar
    if (count > max_lines) {
        const float bar_w = 4.0f;
        const float bar_x = wp.p.x + wp.s.x - bar_w - 2.0f;
        const float bar_y = wp.p.y;
        const float bar_h = wp.s.y;
        const float depth = w->depth + window_manager.ddepth_layer_widget;

        // track
        renderer_2d.batch.add_quad({ bar_x, bar_y }, { bar_w, bar_h }, { 1.0f, 1.0f, 1.0f, 0.1f }, depth);

        // thumb -- position reflects where we are in the buffer
        // scroll_offset=0 means bottom, max_scroll means top
        float max_scroll = (float)(count - max_lines);
        float scroll_t   = 1.0f - glm::clamp(state->scroll_offset / max_scroll, 0.0f, 1.0f);
        float thumb_h    = glm::max(8.0f, bar_h * ((float)max_lines / (float)count));
        float thumb_y    = bar_y + scroll_t * (bar_h - thumb_h);

        renderer_2d.batch.add_quad({ bar_x, thumb_y }, { bar_w, thumb_h }, { 1.0f, 1.0f, 1.0f, 0.5f }, depth);
    }
    
    im_update_cursor_y(w, &wp);

    syn_end_window(handle);
    
}

// 
void syn_help_window() {
    window_handle_t handle = window_manager.get_help_window_handle();
    if (!handle.id) return;
    
    window_t *w = window_manager.get_window(handle);
    if (!w || !w->is_visible()) return;

    syn_begin_window(handle);

    widget_params_t wp;
    im_set_widget_params(w, &wp);
    wp.s.y = w->get_content_size().y - w->im_cursor_y;
    wp.p.y = wp.content_pos.y + w->im_cursor_y;

    std::vector<std::string> lines = dev_tools.get_help_content();
    float line_h = font.get_font_height();
    uint32_t max_lines = (uint32_t)(wp.s.y / line_h);
    uint32_t count = (uint32_t)lines.size();

    uint32_t id = im_widget_hash("__help", w);
    widget_state_t *state = w->im_get_or_create_state(id);
    if (!state) { syn_end_window(handle); return; }

    uint32_t start = 0;
    if (count > max_lines) {
        float max_scroll = (float)(count - max_lines);
        float offset = glm::clamp(state->scroll_offset, 0.0f, max_scroll);
        start = (uint32_t)offset;
    }

    uint32_t display_count = (count > start) ? std::min(count - start, max_lines) : 0;

    font.set_depth(w->depth + window_manager.ddepth_layer_text);
    font.set_color(glm::vec4(1.0f));

    for (uint32_t i = 0; i < display_count; i++) {
        font.render_text_clipped(wp.p.x + 4.0f,
                                  wp.p.y + i * line_h + line_h,
                                  wp.s.x - 8.0f,
                                  "%s", lines[start + i].c_str());
    }

    // scroll
    if (w->im_is_hovered(wp.p, wp.s) && w->im_scroll_delta != 0.0f) {
        float max_scroll = (float)(count > max_lines ? count - max_lines : 0);
        state->scroll_offset = glm::clamp(state->scroll_offset + w->im_scroll_delta, 0.0f, max_scroll);
        w->im_scroll_delta = 0.0f;
    }

    syn_end_window(handle);    
    
}

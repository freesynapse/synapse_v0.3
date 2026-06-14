
#include <memory.h>

#include "c_api.h"

#include "glfw_window.h"
#include "event/input_manager.h"
#include "event/event_handler.h"
#include "renderer/renderer.h"
#include "renderer/shader/shader_library.h"
#include "renderer/mesh/mesh_library.h"
#include "utils/log.h"
#include "utils/time_step.h"
#include "utils/scope_timer.h"
#include "utils/math_utils.h"


//---------------------------------------------------------------------------------------
// globals

// engine core
glfw_window_t               root_window;
gl_api_t                    api;
input_handler_t             input;
events_t                    events;
renderer_t                  renderer;
renderer_2d_t               renderer_2d;
shader_library_t            shader_lib;
texture_library_t           tex_lib;
material_library_t          mat_lib;
mesh_library_t              mesh_lib;
cubemap_library_t           cubemap_lib;
entity_library_t            entity_lib;
time_step_t                 time_step;
asset_manager_t             assets;
window_manager_t            window_manager;
random_t                    rng;

// rendering
font_t                      font;
camera_controller_t         cam;
perspective_camera_t        perspective_camera;
orbit_camera_t              orbit_camera;
orthographic_camera_t       orthographic_camera;

// editing
entity_handle_t             selected_entity_handle = { 0 };


//---------------------------------------------------------------------------------------
// high-level control functions
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

    //
    input.init();

    // 
    rng.init();
    
    //
    api.init();
    api.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });

    //
    renderer.init();
    // renderer.create_scene_framebuffer();
    renderer_2d.init();

    //
    tex_lib.init();
    mat_lib.init();
    cubemap_lib.init();

    font.init("../assets/font/JetBrainsMono-Regular.ttf", 16);
    font.set_color(glm::vec4(1.0f));

    // select main rendering mode
    switch (_mode) {
        case SYN_MODE_2D: syn_mode_2d(); SYN_INFO("SYN_MODE_2D enabled.\n"); break;
        case SYN_MODE_3D: syn_mode_3d(); SYN_INFO("SYN_MODE_3D enabled.\n"); break;
        default: SYN_FATAL_ERROR("unknown rendering mode selected.\n");
    }

    window_manager.init();

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
// accessors and helpers
//
void syn_set_window_pos_quadrant(int _quadrant)
{
    int xoffset, yoffset;
    glfwGetWindowPos(root_window.m_window_ptr, &xoffset, &yoffset);
    glm::ivec2 screen_dim = root_window.m_screen_dim;
    glm::ivec2 win_dim = root_window.get_window_dims();
    switch (_quadrant) {
        case UPPER_LEFT:    xoffset =                        0;  yoffset =                        0;    break;
        case UPPER_RIGHT:   xoffset = screen_dim.x - win_dim.x;  yoffset =                        0;    break;
        case LOWER_LEFT:    xoffset =                        0;  yoffset = screen_dim.y - win_dim.y;    break;
        case LOWER_RIGHT:   xoffset = screen_dim.x - win_dim.x;  yoffset = screen_dim.y - win_dim.y;    break;
        default: SYN_WARNING("unknown screen quadrant (%d).\n", _quadrant);
    }
    root_window.set_window_position(xoffset, yoffset);

}

//
void syn_load_assets(const char *_asset_file)
{
    scope_timer_t t;
    assets.load_manifest(_asset_file);

    // bake hdr maps if skybox is loaded
    if (renderer.m_skybox.is_active) {
        SYN_INFO("baking HDR irradiance and specular maps.\n");
        renderer.bake_irradiance_hdr();
        renderer.bake_specular_hdr();
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
        if      (strcmp(type, "viewport") == 0)  syn_create_viewport_window(name, abs_pos, abs_size);
        else if (strcmp(type, "log") == 0)       syn_create_log_window(name, abs_pos, abs_size);
        else if (strcmp(type, "hierarchy") == 0) syn_create_hierarchy_window(name, abs_pos, abs_size);
        else if (strcmp(type, "transform") == 0) syn_create_transform_window(name, abs_pos, abs_size);
        else if (strcmp(type, "material") == 0)  syn_create_material_window(name, abs_pos, abs_size);            
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
    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_viewport_window(handle);    
    window_t *w = window_manager.get_window(handle);
    w->create_framebuffer();
    
}

// 
void syn_create_log_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name = _name;
    win.position = _pos;
    win.size = _size;

    widget_t text_area;
    text_area.type = widget_type_t::TEXT_AREA;
    text_area.anchor = widget_anchor_t::TOP_LEFT;
    text_area.position = glm::vec2(0.0f, 0.0f);
    text_area.size = glm::vec2(win.size.x - 10.0f, win.size.y - win.title_bar_height - 5.0f);
    text_area.consumes_click = false;

    text_area.get_lines = [](text_area_line_t * _lines, uint32_t _max_lines) -> uint32_t {
        uint32_t count = std::min(syn_log_buffer.count, _max_lines);
        for (uint32_t i = 0; i < count; i++) {
            const log_entry_t &e = syn_log_buffer.get(syn_log_buffer.count - count + i);
            strncpy(_lines[i].text, e.msg, SYN_LOG_LINE_LEN - 1);
            switch (e.level) {
                case log_level_t::WARNING:  _lines[i].color = { 1.0f, 0.8f, 0.0f, 1.0f }; break;
                case log_level_t::ERROR:    _lines[i].color = { 1.0f, 0.3f, 0.3f, 1.0f }; break;
                case log_level_t::DEBUG:    _lines[i].color = { 0.4f, 1.0f, 0.4f, 1.0f }; break;
                default:                    _lines[i].color = { 0.9f, 0.9f, 0.9f, 1.0f }; break;
            }
        }
        return count;
    };

    text_area.on_resize = [](widget_t *_self, const glm::vec2 &_content_size) {
        _self->size = glm::vec2(_content_size.x - 8.0f, _content_size.y - 5.0f);
    };

    text_area.on_scroll = [](widget_t *_self, float _delta) {
        _self->scroll_offset = glm::clamp(_self->scroll_offset + _delta, 0.0f, (float)_self->scroll_max_lines);
    };
    
    window_handle_t handle = window_manager.add_window(win);
    window_t *lw = window_manager.get_window(handle);
    lw->add_widget(text_area);
    lw->on_resize();

    window_manager.set_log_window_handle(handle);
    
}

// 
void syn_create_hierarchy_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name = _name;
    win.position = _pos;
    win.size = _size;
    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_hierarchy_window_handle(handle);
    window_t *hw = window_manager.get_window(handle);

    // entity hierarchy
    widget_t hier;
    hier.type               = widget_type_t::HIERARCHY;
    hier.anchor             = widget_anchor_t::TOP_LEFT;
    hier.position           = glm::vec2(0.0f, 0.0f);
    hier.size               = glm::vec2(_size.x, _size.y - hw->title_bar_height);
    hier.hierarchy_widget.selected = &selected_entity_handle;
    hier.on_resize = [](widget_t *_w, const glm::vec2 &_content_size) {
        _w->size = _content_size;
    };
    hier.on_scroll = [](widget_t *_w, float _delta) {
        _w->hierarchy_widget.scroll_offset = glm::max(0.0f, _w->hierarchy_widget.scroll_offset - _delta);
    };
    
    hw->add_widget(hier);
    hw->on_resize();
    
}

// 
void syn_create_transform_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name = _name;
    win.position = _pos;
    win.size = _size;
    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_transform_window_handle(handle);
    window_t *tw = window_manager.get_window(handle);

    float w         = _size.x;
    float field_h   = 20.0f;
    float field_w   = w * 0.2f;
    float label_x   = w * 0.15f;

    const char *labels[] = { "px ", "py ", "pz ", "rx ", "ry ", "rz ", "sx ", "sy ", "sz " };
    for (int i = 0; i < 9; i++) {
        widget_t f;
        f.type              = widget_type_t::FLOAT_FIELD;
        f.anchor            = widget_anchor_t::TOP_LEFT;
        f.position          = glm::vec2(label_x, i * (field_h + 2.0f));
        f.size              = glm::vec2(field_w, field_h);
        f.text              = labels[i];
        f.float_field.min   = (i >= 6) ? 0.001f : -10000.0f;
        f.float_field.max   = 10000.0f;
        f.float_field.on_change = [i](float _v) {
            if (!selected_entity_handle.is_valid()) return;
            entity_t *e = entity_lib.get_entity(selected_entity_handle);
            if (!e) return;
            if      (i < 3) e->t_position[i % 3] = _v;
            else if (i < 6) e->t_rotation[i % 3] = _v;
            else            e->t_scale[i % 3]     = _v;
            e->transform = entity_t::make_transform(e->t_position, e->t_rotation, e->t_scale);
        };
        tw->add_widget(f);
    }
    tw->on_resize();
    
}

// 
void syn_create_material_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size)
{
    window_t win;
    win.name = _name;
    win.position = _pos;
    win.size = _size;
    window_handle_t handle = window_manager.add_window(win);
    window_manager.set_material_window_handle(handle);
    window_t *mw = window_manager.get_window(handle);

    float w       = _size.x;
    float field_h = 20.0f;
    float field_w = w * 0.2f;
    float label_x = w * 0.15f;

    const char *labels[] = { "r", "g", "b", "rough", "metal", "ao", "tiling" };
    float maxvals[]      = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 100.0f };

    for (int i = 0; i < 7; i++) {
        widget_t f;
        f.type              = widget_type_t::FLOAT_FIELD;
        f.anchor            = widget_anchor_t::TOP_LEFT;
        f.position          = glm::vec2(label_x, i * (field_h + 2.0f));
        f.size              = glm::vec2(field_w, field_h);
        f.text              = labels[i];
        f.float_field.min   = 0.0f;
        f.float_field.max   = maxvals[i];
        f.float_field.on_change = [i](float _v) {
            if (!selected_entity_handle.is_valid()) return;
            entity_t *e = entity_lib.get_entity(selected_entity_handle);
            if (!e) return;
            material_internal_t *mat = mat_lib.get_material(e->material_handle);
            if (!mat || mat->data_size < sizeof(material_pbr_payload_t)) return;
            material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;
            switch (i) {
                case 0: pbr->albedo_color.r = _v; break;
                case 1: pbr->albedo_color.g = _v; break;
                case 2: pbr->albedo_color.b = _v; break;
                case 3: pbr->roughness      = _v; break;
                case 4: pbr->metallic       = _v; break;
                case 5: pbr->ao             = _v; break;
                case 6: pbr->tiling_factor  = _v; break;
            }
        };
        mw->add_widget(f);
    }
    mw->on_resize();
    
}

// 
entity_handle_t _pick_entity(const glm::vec2 &_screen_pos)
{
    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return { 0 };

    glm::vec2 vp_pos  = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();

    if (_screen_pos.x < vp_pos.x || _screen_pos.x > vp_pos.x + vp_size.x ||
        _screen_pos.y < vp_pos.y || _screen_pos.y > vp_pos.y + vp_size.y)
        return { 0 };

    ray_t ray = ray_from_screen(_screen_pos, vp_pos, vp_size, cam.get_view_matrix(), cam.get_projection_matrix());
    entity_handle_t closest_entity = { 0 };
    float closest_t = FLT_MAX;

    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        entity_t *e = entity_lib.get_entity_from_index(i);
        if (!e->is_active) continue;

        mesh_internal_t *mesh = mesh_lib.get_mesh(e->mesh_handle);
        if (!mesh) continue;

        glm::mat4 inv = glm::inverse(e->transform);
        ray_t local_ray;
        local_ray.origin    = glm::vec3(inv * glm::vec4(ray.origin, 1.0f));
        local_ray.direction = glm::normalize(glm::vec3(inv * glm::vec4(ray.direction, 0.0f)));

        float t;
        if (ray_aabb_intersect(local_ray, mesh->aabb_min, mesh->aabb_max, t)) {
            if (t < closest_t) {
                closest_t = t;
                closest_entity = { i + 1 };
            }
        }
    }

    return closest_entity;
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
    
    __was_prerender_called = true;
    
}

// 
void syn_render_begin_3d()
{
    syn_prerender();
    
    // 
    window_t *viewport = window_manager.get_viewport_window();
    if (viewport && viewport->has_frambuffer()) {
        framebuffer_t *fbo = api.fbo_handler.get_framebuffer(viewport->get_framebuffer_handle());
        if (fbo) {
            fbo->bind();
            
            // set viewport to drawable area
            glm::vec2 content_size = viewport->get_content_size();
            glViewport(0, 0, (size_t)content_size.x, (size_t)content_size.y);

            api.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
            api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }

    // perspective_camera.update(time_step.dt);
    cam.update(time_step.dt);

}

//
void syn_render_end_3d()
{
    // render AABB around selected entity
    if (selected_entity_handle.is_valid()) {
        entity_t *e = entity_lib.get_entity(selected_entity_handle);
        if (e) {
            bool prev = renderer.m_debug.show_bounding_boxes;
            renderer.m_debug.show_bounding_boxes = true;
            renderer.render_debug_bounding_box_entities(e);
            renderer.m_debug.show_bounding_boxes = prev;
        }
    }
    
    //
    renderer.render_debug_orientation_obj();

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
    api.set_clear_color(0.2f, 0.2f, 0.2f, 1.0f);
    api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    // ui rendering
    api.clear_depth_buffer();
    window_manager.draw_windows();

    // 
    renderer.draw_perf_stats();
    renderer.draw_notifications();
    font.end_render_block(false);
    
    //
    root_window.post_render();
    events.process_events();
    time_step.update();
    time_step.calculate_fps();

    __was_prerender_called = false;
    
}


#include "synapse.h"


entity_handle_t helmet;
entity_handle_t sphere;

//
void handle_input()
{
    if (input.was_key_pressed(SYN_KEY_F)) {
        event_t e;
        e.type = event_type_t::WINDOW_TOGGLE_FULLSCREEN;
        events.dispatch_event(e);
    }

    if (input.was_key_pressed(SYN_KEY_TAB)) {
        event_t e;
        e.type = event_type_t::WINDOW_TOGGLE_FROZEN_CURSOR;
        events.dispatch_event(e);
    }

    // TODO :   refactor and abstract away
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

// test code
void render(float _dt)
{
    renderer.render_skybox();

    // renderer.cmd_submit_mesh(helmet.mesh_handle, helmet.material_handle, helmet.transform);
    renderer.cmd_submit_entity(helmet);
    renderer.cmd_submit_entity(sphere);
    renderer.cmd_flush();

    if (renderer.m_debug.show_normals || renderer.m_debug.show_tangents) {
        entity_t *e = entity_lib.get_entity(sphere);
        renderer.render_debug_normals(e->mesh_handle, e->transform);
    }

    if (renderer.m_debug.show_bounding_boxes) {
        renderer.render_debug_bounding_box_entities();
    }

    if (renderer.m_debug.show_grid) {
        renderer.render_debug_grid(-2.0f);
    }

}

void setup_lights()
{
    light_t key;
    key.position = glm::vec4(2.0f, 4.0f, 2.0f, 0.0f);   // .w = 0 --> point light
    key.color = glm::vec4(1.0f, 0.9f, 0.8f, 150.0f);
    renderer.set_light(0, key);

    light_t fill;
    fill.position = glm::vec4(-3.0f, 2.0f, 1.0f, 0.0f); // .w = 0 --> point light
    fill.color = glm::vec4(0.8f, 0.9f, 1.0f, 50.0f);
    renderer.set_light(1, fill);

    light_t rim;
    rim.position = glm::vec4(0.0f, 2.0f, -4.0f, 0.0f);  // .w = 0 --> point light
    rim.color = glm::vec4(1.0f, 1.0f, 1.0f, 200.0f);
    renderer.set_light(2, rim);

    light_t sun;
    sun.position = glm::vec4(0, 0, 0, 1.0f);            // .w = 1 --> directional light
    sun.direction = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f); // Top-down angle
    sun.color = glm::vec4(1.0f, 1.0f, 0.9f, 1.0f); // Sun doesn't need 100+ intensity
    renderer.set_light(3, sun);

    renderer.update_lighting_ubo();

}

//
int main()
{
    // syn_init("synapse v0.3", 0, 0, SYN_MODE_3D);
    syn_init("synapse v0.3", 2000, 1400, SYN_MODE_3D);
    root_window.set_exit_key(SYN_KEY_ESCAPE);
    syn_set_window_pos_quadrant(UPPER_RIGHT);

    syn_load_assets("../assets/manifests/helmet_test.syn");
    helmet = assets.get_entity("helmet");

    //
    time_step.fps_limit = 60.0f;
    perspective_camera.m_position = { 0.0f, 2.0f, 20.0f };

    setup_lights();

    mesh_handle_t sphere_mesh = generate_uv_sphere(1.0f, 36, 18);
    material_handle_t chrome_mat = assets.get_material("gold");
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
    // glm::mat4 transform(1.0f);

    sphere = entity_lib.create_entity("test_sphere", sphere_mesh, chrome_mat, transform);

    // perspective_camera.set_x_angle(320.0f);
    // perspective_camera.set_y_angle(30.0f);
    // perspective_camera.set_position({ 2.7f, 2.0f, 2.7f });
    orbit_camera.m_orbit_speed = 0.5f;
    orbit_camera.m_x_angle = 23.0f;
    orbit_camera.m_y_angle = 82.0f;
    orbit_camera.m_radius = 3.0f;

    window_t win1;
    win1.name = "test window 1";
    win1.position = { 100.0f, 700.0f };
    win1.size = { 400.0f, 400.0f };
    window_handle_t win1_handle = window_manager.add_window(win1);
    
    window_t win2;
    win2.name = "test window 2";
    win2.position = { 1200.0f, 150.0f };
    win2.size = { 400.0f, 400.0f };
    win2.set_focused(true);
    window_handle_t win2_handle = window_manager.add_window(win2);

    window_t viewport;
    viewport.name = "Viewport";
    viewport.position = glm::vec2(100.0f, 100.0f);// glm::vec2(0.0f);
    viewport.size = glm::vec2(600.0f);//root_window.window_fdims();
    window_handle_t viewport_handle = window_manager.add_window(viewport);
    window_manager.set_viewport_window(viewport_handle);    
    window_t *vp = window_manager.get_window(viewport_handle);
    vp->create_framebuffer();

    window_t log_win;
    log_win.name = "Log";
    log_win.position = glm::vec2(10.0f, 400.0f);
    log_win.size = glm::vec2(600.0f, 300.0f);
    widget_t log_area;
    log_area.type = widget_type_t::TEXT_AREA;
    log_area.anchor = widget_anchor_t::TOP_LEFT;
    log_area.position = glm::vec2(0.0f, 0.0f);
    log_area.size = glm::vec2(log_win.size.x - 10.0f, log_win.size.y - log_win.title_bar_height - 5.0f);
    log_area.get_lines = [](text_area_line_t * _lines, uint32_t _max_lines) -> uint32_t {
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
    log_area.on_resize = [](widget_t *_self, const glm::vec2 &_content_size) {
        _self->size = glm::vec2(_content_size.x - 8.0f, _content_size.y - 5.0f);
    };
    window_handle_t log_handle = window_manager.add_window(log_win);
    window_t *lw = window_manager.get_window(log_handle);
    lw->add_widget(log_area);
    lw->on_resize();
    
    //
    while (!root_window.should_close()) {

        handle_input();

        // RENDERING 3D tests
        syn_render_begin_3d();
        render(time_step.dt);
        syn_render_end_3d();
    }

    syn_shutdown();

    return 0;

}

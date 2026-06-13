
#include "synapse.h"


entity_handle_t helmet;
entity_handle_t sphere;

void setup_lights();
void render(float _dt);
void handle_input();

//
int main()
{
    syn_init("synapse v0.3", 0, 0, SYN_MODE_3D);
    // syn_init("synapse v0.3", 2000, 1400, SYN_MODE_3D);
    root_window.set_exit_key(SYN_KEY_W, SYN_MOD_CTRL);
    // syn_set_window_pos_quadrant(UPPER_RIGHT);

    syn_load_assets("../assets/manifest.syn");
    helmet = assets.get_entity("helmet");

    //
    time_step.fps_limit = 60.0f;
    perspective_camera.m_position = { 0.0f, 2.0f, 20.0f };

    setup_lights();

    mesh_handle_t sphere_mesh = generate_uv_sphere(1.0f, 36, 18);
    material_handle_t chrome_mat = assets.get_material("gold");
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));

    sphere = entity_lib.create_entity("test_sphere", sphere_mesh, chrome_mat, transform);

    orbit_camera.m_orbit_speed = 0.5f;
    orbit_camera.m_x_angle = 23.0f;
    orbit_camera.m_y_angle = 82.0f;
    orbit_camera.m_distance = 3.0f;
    // cam.set_mode(camera_mode_t::ORBIT);

    //
    syn_load_ui_layout("../assets/layout.syn");
    
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


//
void handle_input()
{
    if (input.was_key_pressed(SYN_KEY_F)) {
        event_t e;
        e.type = event_type_t::WINDOW_TOGGLE_FULLSCREEN;
        events.dispatch_event(e);
    }

    // 
    if (input.was_key_pressed(SYN_KEY_TAB)) {
        cam.toggle_mode();
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

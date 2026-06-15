
#include "synapse.h"


entity_handle_t helmet;
entity_handle_t sphere;
entity_handle_t test_cube;

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

    mesh_handle_t sphere_mesh = mesh_generator.create_uv_sphere_mesh();
    material_handle_t sphere_material = assets.get_material("chrome");
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 2.0f));

    sphere = entity_lib.create_entity("test_sphere", sphere_mesh, sphere_material, transform);

    // 
    orbit_camera.m_orbit_speed = 0.5f;
    orbit_camera.m_x_angle = 45.0f;
    orbit_camera.m_y_angle = 60.0f;
    orbit_camera.m_distance = 14.0f;
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
void render(float _dt)
{
    // renderer.render_skybox();

    entity_t *pool = entity_lib.get_pool();
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        if (pool[i].is_active) {
            renderer.cmd_submit_entity({ i + 1 });
        }
    }
    
    renderer.cmd_flush();

    if (renderer.debug.show_normals || renderer.debug.show_tangents) {
        entity_t *e = entity_lib.get_entity(sphere);
        renderer.render_debug_normals(e->mesh_handle, e->transform);
    }

    if (renderer.debug.show_bounding_boxes) {
        renderer.render_debug_bounding_box_entities();
    }

    if (renderer.debug.show_grid) {
        renderer.render_debug_grid();
    }

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

// 
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

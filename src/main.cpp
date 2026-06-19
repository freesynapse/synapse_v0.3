
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
    
    entity_t *e = entity_lib.get_entity(sphere);
    if (e) {
        e->t_position = { 0.0f, 1.0f, 2.0f };
        e->t_rotation = { 0.0f, 0.0f, 0.0f };
        e->t_scale    = { 1.0f, 1.0f, 1.0f };
        e->mesh_primitive_type = primitive_type_t::SPHERE_UV;
        e->mesh_params[0] = 36.0f;
        e->mesh_params[1] = 18.0f;
        e->mesh_param_count = 2;
        e->manifest_material_name = "chrome";
    }
    
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
    
}

//
void handle_input()
{}

// 
void setup_lights()
{
    // light_t key;
    // key.position = glm::vec4(2.0f, 4.0f, 2.0f, 0.0f);   // .w = 0 --> point light
    // key.color = glm::vec4(1.0f, 0.9f, 0.8f, 150.0f);
    // renderer.set_light(0, key);

    // light_t fill;
    // fill.position = glm::vec4(-3.0f, 2.0f, 1.0f, 0.0f); // .w = 0 --> point light
    // fill.color = glm::vec4(0.8f, 0.9f, 1.0f, 50.0f);
    // renderer.set_light(1, fill);

    // light_t rim;
    // rim.position = glm::vec4(0.0f, 2.0f, -4.0f, 0.0f);  // .w = 0 --> point light
    // rim.color = glm::vec4(1.0f, 1.0f, 1.0f, 200.0f);
    // renderer.set_light(2, rim);

    light_t sun;
    sun.position = glm::vec4(0, 0, 0, 1.0f);            // .w = 1 --> directional light
    sun.direction = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f); // Top-down angle
    sun.color = glm::vec4(1.0f, 1.0f, 0.9f, 1.0f); // Sun doesn't need 100+ intensity
    // renderer.set_light(3, sun);
    renderer.set_light(0, sun);

    renderer.update_lighting_ubo();

}

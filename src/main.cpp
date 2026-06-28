
#include "synapse.h"
#include "mplc/mplc.h"


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

    syn_load_assets("../assets/manifest.syn");
    syn_load_ui_layout("../assets/layout.syn");

    //
    time_step.fps_limit = 60.0f;
    perspective_camera.m_position = { 0.0f, 2.0f, 20.0f };

    setup_lights();
    renderer.enable_shadows(true);
    renderer.set_shadow_ortho(20.0f, 0.1f, 100.0f);
    
    // get/create entities
    helmet = assets.get_entity("helmet");
    sphere = editor.create_entity_from_primitive(primitive_type_t::SPHERE_UV, "test_sphere", { 0.0f, 1.0f, 2.0f }, "chrome");
    editor.create_entity_from_primitive(primitive_type_t::PLANE, "ground", { 0.0f, 0.1f, 0.0f }, "rough_plastic");
    
    //
    while (!root_window.should_close()) {

        handle_input();

        //
        syn_render_begin_3d();
        render(time_step.dt);
        syn_render_end_3d();

        // immediate mode ui
        syn_im_begin();
    
        editor.draw_color_picker_window();
        editor.draw_material_window();
        editor.draw_transform_window();
        editor.draw_hierarchy_window();
        editor.draw_texture_select_window();
        editor.draw_create_primitive_window();
        
        syn_log_window();
        syn_help_window();
    
        syn_im_end();

        syn_frame_end();
    }

    syn_shutdown();
    mplc.shutdown();
    
    return 0;

}

//
void render(float _dt)
{
    entity_t *ent_pool = entity_lib.get_pool();
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        if (ent_pool[i].is_active) {
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
    // sun.direction = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f); // Top-down angle
    // sun.direction = glm::vec4(0.5f, 0.8f, 0.34f, 0.0f);
    sun.direction = renderer.skybox_find_sun_direction();
    sun.color = glm::vec4(1.0f, 1.0f, 0.9f, 1.0f); // Sun doesn't need 100+ intensity
    // renderer.set_light(3, sun);
    renderer.set_light(0, sun);

    renderer.update_lighting_ubo();

}

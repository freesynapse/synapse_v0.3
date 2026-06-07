
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

// rendering
font_t                      font;
perspective_camera_t        perspective_camera;
orbit_camera_t              orbit_camera;
orthographic_camera_t       orthographic_camera;


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
    api.init();
    api.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });

    //
    renderer.init();
    renderer.create_scene_framebuffer();
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
void syn_mode_2d()
{
    glm::ivec2 dims = root_window.window_dims();
    orthographic_camera = orthographic_camera_t((float)dims.x / (float)dims.y);
}

//
void syn_mode_3d()
{
    glm::ivec2 dims = root_window.window_dims();
    orbit_camera.init(60.0f, dims.x, dims.y, 0.1f, 1000.0f);
    perspective_camera.init(60.0f, dims.x, dims.y, 0.1f, 1000.0f);

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
    glm::ivec2 win_dim = root_window.window_dims();
    switch (_quadrant) {
        case UPPER_LEFT:    xoffset =                        0;  yoffset =                        0;    break;
        case UPPER_RIGHT:   xoffset = screen_dim.x - win_dim.x;  yoffset =                        0;    break;
        case LOWER_LEFT:    xoffset =                        0;  yoffset = screen_dim.y - win_dim.y;    break;
        case LOWER_RIGHT:   xoffset = screen_dim.x - win_dim.x;  yoffset = screen_dim.y - win_dim.y;    break;
        default: SYN_WARNING("unknown screen quadrant (%d).\n", _quadrant);
    }
    root_window.set_window_position(xoffset, yoffset);

}


//---------------------------------------------------------------------------------------
// rendering loop functions
//
void syn_render_begin_3d()
{
    root_window.pre_render();
    renderer.reset_perf_counters();

    // perspective_camera.update(time_step.dt);
    orbit_camera.update(time_step.dt);

    // bind the scene framebuffer, everything is rendered to this buffer
    renderer.bind_scene_fbuffer();

    api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.render_skybox();

}

//
void syn_render_end_3d()
{
    //
    renderer.draw_debug_orientation_obj();

    // draw_perf_overlay does NOT contain font.start_/.end_render_block()
    renderer.record_frame_time(time_step.dt * 1000.0f);
    renderer.draw_perf_stats();
    renderer.draw_notifications();

    //
    syn_render_end();

}

//
void syn_render_end()
{
    // here we call end_render_block, effectively rendering all text with one call
    font.end_render_block(false);

    // everything is drawn, render the screen NDC quad
    renderer.render_scene_fbuffer();

    // ui rendering
    api.clear_depth_buffer();
    
    window_manager.draw_windows();
    // reset font rendring depth
    font.set_depth(-1.0f);
    
    //
    root_window.post_render();
    events.process_events();
    time_step.update();
    time_step.calculate_fps();

}

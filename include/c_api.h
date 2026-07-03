#ifndef __C_API_H
#define __C_API_H

#include <glm/glm.hpp>

#include "glfw_window.h"
#include "gl_api.h"
#include "editor.h"
#include "dev_tools.h"
#include "event/input_manager.h"
#include "event/event_handler.h"
#include "renderer/renderer.h"
#include "renderer/renderer_2d.h"
#include "renderer/shader/shader_library.h"
#include "renderer/material/texture_library.h"
#include "renderer/material/material_library.h"
#include "renderer/material/cubemap_library.h"
#include "renderer/mesh/mesh_library.h"
#include "renderer/mesh/mesh_generator.h"
#include "renderer/entity/entity_library.h"
#include "renderer/font/font.h"
#include "renderer/camera/camera_controller.h"
#include "renderer/camera/orbit_camera.h"
#include "renderer/camera/perspective_camera.h"
#include "renderer/camera/orthographic_camera.h"
#include "renderer/UI/window/window_manager.h"
#include "utils/time_step.h"
#include "utils/asset_manager.h"
#include "utils/random.h"
#include "utils/file_io_handler.h"


/*  This file defines the C API style bindings for synapse, to be used from now on.
    The style is trying to mimick raylib where possible, where most parameters are
    abstracted away. Core engine objects are stack-allocated.
 */


//---------------------------------------------------------------------------------------
// definitions and enums
//
#define UPPER_LEFT      1
#define UPPER_RIGHT     2
#define LOWER_LEFT      3
#define LOWER_RIGHT     4


//---------------------------------------------------------------------------------------
// globals

// engine core
extern glfw_window_t                root_window;
extern gl_api_t                     api;
extern input_handler_t              input;
extern events_t                     events;
extern renderer_t                   renderer;
extern renderer_2d_t                renderer_2d;
extern time_step_t                  time_step;
extern file_io_handler_t            file_io_handler;
extern shader_library_t             shader_lib;
extern texture_library_t            tex_lib;
extern material_library_t           mat_lib;
extern mesh_library_t               mesh_lib;
extern cubemap_library_t            cubemap_lib;
extern entity_library_t             entity_lib;
extern asset_manager_t              assets;
extern window_manager_t             window_manager;
extern random_t                     rng;
extern mesh_generator_t             mesh_generator;
extern editor_t                     editor;
extern dev_tools_t                  dev_tools;

// rendering
extern font_t                       font;
extern camera_controller_t          cam;
extern perspective_camera_t         perspective_camera;
extern orbit_camera_t               orbit_camera;
extern orthographic_camera_t        orthographic_camera;

// editing
extern entity_handle_t              selected_entity_handle;


//---------------------------------------------------------------------------------------
/*  _name, _width and _height of root_window. _mode dictates which cameras are initiated
    by default, where SYN_MODE_2D creates a orthographic camera and SYN_MODE_3D
    creates a perspective camera and an orbit camera.
 */

//---------------------------------------------------------------------------------------
// high-level control
//
void syn_init(const char *_name, int _width, int _height);
void syn_shutdown();
void syn_enable_2d();
void syn_enable_assets();
void syn_enable_font(const char *_filename, int _px_size);
void syn_enable_window_manager();
void syn_enable_editor();
void syn_enable_mplc();
void syn_enable_mode_2d();
void syn_enable_mode_3d();


//---------------------------------------------------------------------------------------
// rendering loop functions
//
void syn_begin_frame();
void syn_end_frame();

// optional per-frame subsystem flushes
// void syn_prerender();
void syn_render_begin_3d();
void syn_render_end_3d();
void syn_flush_2d();
void syn_flush_font(bool _use_depth_test=false);
void syn_flush_windows();

// void syn_render_end();
// void syn_frame_end();


//---------------------------------------------------------------------------------------
// ui creation and ui asset loader
//
void syn_load_assets(const char *_asset_file="assets.syn");
void syn_load_ui_layout(const char *_filepath);
void syn_save_ui_layout(const char *_filepath);
void syn_create_viewport_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_log_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_perf_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_hierarchy_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_transform_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_material_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_primitive_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_texture_select_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_help_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);
void syn_create_color_picker_window(const char *_name, const glm::vec2 &_pos, const glm::vec2 &_size);


//---------------------------------------------------------------------------------------
// immediate mode ui
// 
void syn_im_begin();
void syn_im_end();
window_t *syn_im_begin_window(window_handle_t _handle);
void syn_im_end_window(window_handle_t _handle);
void syn_im_begin_row(window_handle_t _handle, const std::vector<float> &_ratios);
void syn_im_end_row(window_handle_t _handle);

void syn_im_label(window_handle_t _handle, const char *_text);
bool syn_im_button(window_handle_t _handle, const char *_text);
void syn_im_float_field(window_handle_t _handle, const char *_label, float *_value, float _min=-FLT_MAX, float _max=FLT_MAX);
void syn_im_color_picker_sv(window_handle_t _handle, float *_hue, float *_saturation, float *_value);
void syn_im_color_picker_hue(window_handle_t _handle, float *_hue);
void syn_im_color_swatch(window_handle_t _handle, float *_r, float *_g, float *_b);
bool syn_im_list(window_handle_t _handle, const char **_items, uint32_t _count, int *_selected_index, int *_hovered_index=nullptr, float _max_height=0.0f);
void syn_im_tex_quad(window_handle_t _handle, texture_handle_t _tex_handle, const glm::vec2 &_size, const glm::vec2 &_pos=glm::vec2(-1.0f));
void syn_im_tex_quad_raw(window_handle_t _handle, GLuint _tex_id, const glm::vec2 &_size, const glm::vec2 &_pos=glm::vec2(-1.0f));

// 
void syn_log_window();
void syn_perf_window();
void syn_help_window();

//---------------------------------------------------------------------------------------
// rendering functions
//
#define syn_render_text(float_x, float_y, ...) font->addString((float_x), (float_y), __VA_ARGS__)





#endif // __C_API_H

#ifndef __C_API_H
#define __C_API_H

#include <glm/glm.hpp>

#include "glfw_window.h"
#include "gl_api.h"
#include "event/input_manager.h"
#include "event/event_handler.h"
#include "renderer/renderer.h"
#include "renderer/renderer_2d.h"
#include "renderer/material/texture_library.h"
#include "renderer/material/material_library.h"
#include "renderer/material/cubemap_library.h"
#include "renderer/mesh/mesh_library.h"
#include "renderer/entity/entity_library.h"
#include "renderer/font/font.h"
#include "renderer/camera/orbit_camera.h"
#include "renderer/camera/perspective_camera.h"
#include "renderer/camera/orthographic_camera.h"
#include "utils/time_step.h"
#include "utils/asset_manager.h"
#include "renderer/UI/window/window_manager.h"
#include "renderer/UI/window/ui_render_batch.h"


/*  This file defines the C API style bindings for synapse, to be used from now on.
    The style is trying to mimick raylib where possible, where most parameters are
    abstracted away. Core engine objects are stack-allocated.
 */


//---------------------------------------------------------------------------------------
// definitions and enums
//
#define SYN_MODE_2D     1
#define SYN_MODE_3D     2

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
extern shader_library_t             shader_lib;
extern texture_library_t            tex_lib;
extern material_library_t           mat_lib;
extern mesh_library_t               mesh_lib;
extern cubemap_library_t            cubemap_lib;
extern entity_library_t             entity_lib;
extern time_step_t                  time_step;
extern asset_manager_t              assets;
extern window_manager_t             window_manager;
extern ui_render_batch_t            ui_render_batch;

// rendering
extern font_t                       font;
extern perspective_camera_t         perspective_camera;
extern orbit_camera_t               orbit_camera;
extern orthographic_camera_t        orthographic_camera;

//---------------------------------------------------------------------------------------
// high-level control functions
//

/*  _name, _width and _height of root_window. _mode dictates which cameras are initiated
    by default, where SYN_MODE_2D creates a orthographic camera and SYN_MODE_3D
    creates a perspective camera and an orbit camera.
 */
void syn_init(const char *_name, int _width, int _height, int _mode);
void syn_load_assets(const char *_asset_file="assets.syn");
void syn_mode_2d();
void syn_mode_3d();
void syn_shutdown();


//---------------------------------------------------------------------------------------
// accessors and helpers
//
void syn_destroy_window();
void syn_set_window_pos_quadrant(int _quadrant);


//---------------------------------------------------------------------------------------
// rendering loop functions
//
void syn_render_begin_3d();
void syn_render_end_3d();
void syn_render_end();


//---------------------------------------------------------------------------------------
// rendering functions
//
#define syn_render_text(float_x, float_y, ...) font->addString((float_x), (float_y), __VA_ARGS__)





#endif // __C_API_H

Synapse UI System — API Reference
Core Types
window_handle_t
cppstruct window_handle_t { uint32_t id = 0; };  // id == 0 is invalid
widget_handle_t — implicit, widgets accessed by index via window_t::get_widget(uint32_t)

Window Manager (window_manager_t)
Global instance: window_manager
Window lifecycle
cppwindow_handle_t add_window(window_t &_window);
void            release_window(window_handle_t _handle);
window_t       *get_window(const window_handle_t &_handle);
Special windows
cppvoid            set_viewport_window(const window_handle_t &_handle);
window_t       *get_viewport_window();
window_handle_t get_viewport_window_handle();
bool            has_viewport_window();

void            set_log_window_handle(const window_handle_t &_handle);
window_handle_t get_log_window_handle();
void            set_properties_window_handle(const window_handle_t &_handle);
window_handle_t get_properties_window_handle();
Focus and interaction
cppvoid            set_focused_window(window_handle_t _handle);
window_handle_t get_focused_window();
window_handle_t get_window_at_pos(const glm::vec2 _pos, window_handle_t _exclude = {0});
Docking
cppvoid dock_as_tab(window_handle_t _new, window_handle_t _target);
void apply_docking(window_handle_t _handle, dock_zone_t _zone, window_handle_t _target);
Layout persistence
cppvoid syn_load_ui_layout(const char *_filepath);
void syn_save_ui_layout(const char *_filepath);

C API — Window Creation
cppwindow_handle_t syn_create_viewport(const char *_name, glm::vec2 _position, glm::vec2 _size);
void            syn_create_log_window(const char *_name, glm::vec2 _position, glm::vec2 _size);

window_t
Public fields
cppstd::string  name;
glm::vec2    position;
glm::vec2    size;
float        depth;
float        title_bar_height;      // default 26.0f

glm::vec4    bg_color;
glm::vec4    fg_color;
glm::vec4    outline_color;
glm::vec4    title_bar_color;
glm::vec4    title_bar_color_focused;

bool         m_is_tab_container;
bool         m_is_tab_child;
bool         m_is_movable;
bool         m_is_resizable;
Tab system
cpp// set automatically by dock_as_tab()
window_handle_t m_tab_children[SYN_WINDOW_MAX_TABS];
uint32_t        m_tab_count;
uint32_t        m_active_tab;
window_handle_t m_tab_parent;
Framebuffer
cppvoid create_framebuffer();
void resize_framebuffer();
void destroy_framebuffer();
bool has_framebuffer();
framebuffer_handle_t get_framebuffer_handle();

glm::vec2 get_content_size();      // size minus title bar
glm::vec2 get_content_position();  // position offset by title bar height
Widgets
cppvoid      add_widget(const widget_t &_widget);
widget_t *get_widget(uint32_t _index);
widget_t *get_widget_at_pos(const glm::vec2 &_pos);
Resize notification
cppvoid on_resize();   // notifies all widgets and resizes framebuffer if present

widget_t
Common fields
cppwidget_type_t   type;               // BUTTON, LABEL, TEXT_AREA
widget_anchor_t anchor;             // TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, CENTER
glm::vec2       position;           // relative to anchor, in content-area space
glm::vec2       size;
glm::vec4       color;
glm::vec4       hover_color;
glm::vec4       outline_color;
std::string     text;               // for BUTTON labels

bool            is_visible;         // default true
bool            is_enabled;         // default true
bool            is_hovered;         // set by window manager
bool            in_title_bar;       // if true, positioned relative to full window, not content area
bool            consumes_click;     // if false, click falls through to window focus/drag (default true)
Callbacks
cppstd::function<void()>                               on_click;
std::function<void(widget_t *, const glm::vec2 &)>  on_resize;   // called with content size
std::function<void(widget_t *, float)>              on_scroll;   // called with y delta
TEXT_AREA specific fields
cppstd::function<uint32_t(text_area_line_t *, uint32_t)> get_lines;
float    scroll_offset;       // 0 = newest (bottom), increases toward oldest
uint32_t scroll_max_lines;    // updated each frame by draw_widgets

text_area_line_t
cppstruct text_area_line_t {
    char      text[SYN_LOG_LINE_LEN];   // SYN_LOG_LINE_LEN = 512
    glm::vec4 color;
};

Log Buffer
Global instance: syn_log_buffer
cppstruct log_ring_buffer_t {
    uint32_t          count;                            // total entries, capped at SYN_LOG_BUFFER_SIZE
    log_entry_t       entries[SYN_LOG_BUFFER_SIZE];     // SYN_LOG_BUFFER_SIZE = 256
    const log_entry_t &get(uint32_t _idx) const;        // oldest-to-newest indexing
};

struct log_entry_t {
    char        msg[SYN_LOG_LINE_LEN];
    log_level_t level;    // INFO, DEBUG, WARNING, ERROR
};

Dock Zones
cppenum class dock_zone_t { NONE, LEFT, RIGHT, TOP, BOTTOM, CENTER };
Drag a window to screen edges for LEFT/RIGHT/TOP/BOTTOM docking. Drag onto another window's center area for CENTER (tab) docking.

Key Bindings
BindingActionCtrl+TabNext tab in focused containerCtrl+Shift+TabPrevious tab in focused containerClick title barFocus + drag windowClick tabSelect tabScroll over TEXT_AREAScroll content

Limits
ConstantValueSYN_MAX_WINDOW_COUNT32SYN_WINDOW_MAX_TABS8SYN_WINDOW_MAX_WIDGET_COUNT(fixed pool per window)SYN_LOG_BUFFER_SIZE256SYN_LOG_LINE_LEN512You said: Show me the typically workflow for snapsen initialization, including layout and asset manifest as a code snippetShow me the typically workflow for snapsen initialization, including layout and asset manifest as a code snippet4:34 PMClaude responded: With a corresponding layout.cpp#include "synapse.h"

int main()
{
    // 1. initialize engine (window, GL, cameras, subsystems)
    syn_init("my app", 0, 0, SYN_MODE_3D);  // 0,0 = fullscreen
    root_window.set_exit_key(SYN_KEY_ESCAPE);

    // 2. load assets (meshes, textures, shaders, materials, entities, skybox)
    syn_load_assets("../assets/manifest.asset");

    // 3. load UI layout (creates and positions all windows)
    syn_load_ui_layout("../assets/layout.syn");

    // 4. optional: per-app camera setup
    orbit_camera.m_orbit_speed = 0.5f;
    orbit_camera.m_x_angle     = 23.0f;
    orbit_camera.m_y_angle     = 82.0f;
    orbit_camera.m_radius      = 3.0f;

    // 5. main loop
    while (!root_window.should_close()) {
        syn_render_begin_3d();

        // submit render commands
        renderer.cmd_submit_entity(my_entity);
        renderer.cmd_flush();

        // optional debug rendering
        renderer.render_debug_grid();
        renderer.render_debug_normals(my_mesh, my_transform);

        syn_render_end_3d();
    }

    syn_shutdown();
    return 0;
}

With a corresponding layout.syn:
#viewport
name        Viewport
position    0.02 0.02
size        0.60 0.75

#log
name        Log
position    0.02 0.79
size        0.96 0.19

#window
name        Properties
position    0.64 0.02
size        0.34 0.75

And manifest.asset following your existing format:
# ==============================================================================
# ASSET MANIFEST - Damaged Helmet with LDR Skybox Scene
# ==============================================================================

# SHADERS
shader pbr_ibl ../assets/shaders/PBR_IBL.glsl

# TEXTURES
texture helmet_albedo ../assets/models/damaged_helmet/albedo.jpg
texture helmet_normal ../assets/models/damaged_helmet/normal.jpg
texture helmet_roughness ../assets/models/damaged_helmet/roughness.jpg
texture helmet_metallic ../assets/models/damaged_helmet/metallic.jpg
texture helmet_ao ../assets/models/damaged_helmet/AO.jpg
texture helmet_emissive ../assets/models/damaged_helmet/emissive.jpg

# MESHES
mesh helmet_mesh ../assets/models/damaged_helmet/DamagedHelmet.gltf

# MATERIALS
material helmet_mat
{
    shader: pbr_ibl
    
    albedo_texture: helmet_albedo
    normal_texture: helmet_normal
    metallic_texture: helmet_metallic
    roughness_texture: helmet_roughness
    ao_texture: helmet_ao
    emissive_texture: helmet_emissive
    
    use_albedo_map: true
    use_normal_map: true
    use_metallic_map: true
    use_roughness_map: true
    use_ao_map: true
    use_emissive_map: true
    
    albedo_color: 1.0 1.0 1.0 1.0
    tiling_factor: 1.0
}

# SKYBOX
skybox main_sky ../assets/textures/skybox/Lycksele2/posx.jpg ../assets/textures/skybox/Lycksele2/negx.jpg ../assets/textures/skybox/Lycksele2/posy.jpg ../assets/textures/skybox/Lycksele2/negy.jpg ../assets/textures/skybox/Lycksele2/posz.jpg ../assets/textures/skybox/Lycksele2/negz.jpg
#skybox main_sky ../assets/textures/skybox/sky/posx.png ../assets/textures/skybox/sky/negx.png ../assets/textures/skybox/sky/posy.png ../assets/textures/skybox/sky/negy.png ../assets/textures/skybox/sky/posz.png ../assets/textures/skybox/sky/negz.png


# -----------------------------------------------------------------------------
# Chrome - Shiny metal (no textures)
# -----------------------------------------------------------------------------
material chrome
{
    shader: pbr_ibl
    
    # No textures - pure material values
    albedo_color: 0.95 0.95 1.0 1.0
    metallic: 1.0
    roughness: 0.05
    ao: 1.0
}

# -----------------------------------------------------------------------------
# Rough Plastic - Non-metallic matte surface
# -----------------------------------------------------------------------------
material rough_plastic
{
    shader: pbr_ibl
    
    albedo_color: 0.8 0.2 0.2 1.0
    metallic: 0.0
    roughness: 0.9
    ao: 1.0
}

# -----------------------------------------------------------------------------
# Gold - Metallic with medium roughness
# -----------------------------------------------------------------------------
material gold
{
    shader: pbr_ibl
    
    albedo_color: 1.0 0.84 0.0 1.0
    metallic: 1.0
    roughness: 0.3
    ao: 1.0
}

# ==============================================================================
# ENTITIES
# ==============================================================================
# Format: entity <name> { <properties> }
# Creates scene objects with mesh, material, and transform

entity helmet
{
    mesh: helmet_mesh
    material: helmet_mat
    position: 0.0 0.0 0.0
    rotation: 90.0 90.0 0.0
    scale: 1.0 1.0 1.0
}


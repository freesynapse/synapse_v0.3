#ifndef __RENDERER_H
#define __RENDERER_H

#include "event/event.h"
#include "renderer/buffers/vertex_array.h"
#include "renderer/renderer_types.h"
#include "renderer/buffers/framebuffer_types.h"
#include "renderer/lighting/lighting_types.h"
#include "renderer/material/cubemap_types.h"
#include "renderer/entity/entity_types.h"
#include "renderer/shadow/shadow_types.h"

#include "mplc/figure_types.h"

//
#define SYN_MAX_RENDER_COMMANDS     1024

//
class renderer_t
{
public:
    friend class renderer_2d_t;

public:
    void init();
    void shutdown();

    //
	void on_resize(const event_t &_e);

	// material ubo
private:
	void init_material_ubo();
	void release_material_ubo();

	// lighting
private:
	void init_lighting_ubo();
	void release_lighting_ubo();
public:
	void update_lighting_ubo();
	void set_light(uint32_t _index, const light_t &_light);

	// skybox
private:
	void init_skybox();
public:
	void set_skybox(const cubemap_handle_t &_handle);
	void set_skybox_render(bool _bool) { m_do_render_skybox = _bool; }
	void toggle_skybox() { m_do_render_skybox = !m_do_render_skybox; }
	void render_skybox();
	glm::vec4 skybox_find_sun_direction();

	// IBL
	void bake_ibl();
	void bake_irradiance_hdr();
	void bake_specular_hdr();
	cubemap_handle_t convert_equirect_to_cubemap(const texture_handle_t &_hdr_tex_handle, uint32_t _tex_size=2048);
	
	// shadow map
	void init_shadow_map();
	void release_shadow_map();
	void render_shadow_pass();
	void enable_shadows(bool _enable) { m_shadow_map.is_active = _enable; }
	void set_shadow_ortho(float _size, float _z_near, float _z_far);
	
	// scene framebuffer functions
	void set_scene_framebuffer(const framebuffer_handle_t &_handle) { m_scene_fbuffer_handle = _handle; }
	void bind_scene_fbuffer(bool _update_viewport=true);
	void unbind_scene_fbuffer();
	void render_scene_fbuffer();

	// render commands
	void cmd_submit_mesh(mesh_handle_t _mesh, material_handle_t _material, const glm::mat4 &_transform);
	void cmd_submit_entity(entity_handle_t _entity_handle);
	void cmd_flush();

	// performance stats
	void reset_perf_counters();
	void record_frame_time(float _dt_ms);
	void show_notification(const std::string &_msg, float _duration_s=4.0f);
	void draw_notifications();

	// ui elements
	void render_ui_transform(const glm::vec3 &_world_pos);
	const glm::mat4 &get_ui_projection_matrix() { return m_ui_projection; }
	void calculate_ui_projection_matrix();
	
	// element inits
private:
    void init_debug_rendering();
    void init_orienatation_obj(uint32_t _size);
    void init_ui_rendering();
    void shutdown_ui_rendering();
    
public:
	void toggle_wireframe();
	void toggle_normals();
	void toggle_tangents();
	void toggle_bounding_boxes();
	void toggle_grid();

	void render_debug_normals(mesh_handle_t _mesh_handle, const glm::mat4 &_transform);
	void render_debug_tangents(mesh_handle_t _mesh_handle, const glm::mat4 &_transform);
	// if _entity is nullptr, AABBs of all entities in the entity library are drawn
	void render_debug_bounding_box_entities(entity_t *_entity=nullptr);
	void render_debug_bounding_boxes(const glm::vec3 &_min, const glm::vec3 &_max, const glm::mat4 &_transform);
	void render_debug_bounding_boxes(entity_t *_entity=nullptr);
	void render_debug_grid(float _y_level=0.0f);
	void render_orientation_obj();


public:
	GLuint m_stored_framebuffer = 0;

	// main, final, framebuffer -- everything renders to this
	uint32_t m_scene_fbuffer_vao;
	shader_handle_t m_scene_fbuffer_shader_handle;
	framebuffer_handle_t m_scene_fbuffer_handle;

	// ubos
	uniform_buffer_t m_material_ubo;
	uniform_buffer_t m_lighting_ubo;

	// lights
	light_internal_t m_lighting_state;

	// skybox
	skybox_t m_skybox;
	bool m_do_render_skybox = true;
	

	// IBL maps
	cubemap_handle_t m_irradiance_map;
	cubemap_handle_t m_prefilter_map;

	// shadow map
	shadow_map_t m_shadow_map;
	shader_handle_t m_shadow_shader_handle;
	
	// render command
	render_command_t m_command_queue[SYN_MAX_RENDER_COMMANDS];
	uint32_t m_command_count;

	// performance stats
	perf_stats_t m_perf_stats; // .draw_calls_per_frame reset in root_window.prerender()
	figure_handle_t m_perf_figure = { 0 };

	// notifications
	struct {
	    std::string msg;
		float display_time = 0.0f;
		float duration = 2.0f;
	} m_notification;

	// ui stuff
	shader_handle_t m_ui_transform_shader_handle;
	vertex_array_t m_ui_transform_vao;
	shader_handle_t m_ui_color_picker_shader_handle;
	glm::mat4 m_ui_projection;
	shader_handle_t m_ui_tex_quad_shader_handle;
    vertex_array_t m_ui_tex_quad_vao;
	
	// debug (vaos and shader handles in debug_state_t)
	debug_state_t debug;
	bool m_debug_initialized = false;

	vertex_array_t m_orientation_obj_vao;
	shader_handle_t m_orientation_obj_shader_handle;
	uint32_t m_orientation_obj_size;

};



#endif // __RENDERER_H

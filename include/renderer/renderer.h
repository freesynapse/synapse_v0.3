#ifndef __RENDERER_EX_H
#define __RENDERER_EX_H

#include "event/event.h"
#include "renderer/buffers/vertex_array.h"
#include "renderer/buffers/framebuffer.h"
#include "renderer/renderer_types.h"
#include "renderer/buffers/framebuffer_types.h"
#include "renderer/lighting/lighting_types.h"
#include "renderer/material/cubemap_types.h"
#include "renderer/entity/entity_types.h"

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
	void shutdown_material_ubo();

	// lighting
private:
	void init_lighting_ubo();
	void shutdown_lighting_ubo();
public:
	void update_lighting_ubo();
	void set_light(uint32_t _index, const light_t &_light);

	// skybox
private:
	void init_skybox();
public:
	void set_skybox(const cubemap_handle_t &_handle);
	void render_skybox();

	// IBL
	void bake_irradiance_hdr();
	void bake_specular_hdr();
	cubemap_handle_t convert_equirect_to_cubemap(const texture_handle_t &_hdr_tex_handle);

	// framebuffer functions
	framebuffer_handle_t create_framebuffer(const color_format_t& _format=color_format_t::RGBA16F,
				                            const glm::ivec2& _size=glm::ivec2(0),
				                            size_t _n_drawbuffers=1,
				                            bool _use_depthbuffer=true,
				                            const std::string& _name="");
	framebuffer_t *get_framebuffer(const framebuffer_handle_t &_handle);
	void create_scene_framebuffer();
	void bind_scene_fbuffer();
	void render_scene_fbuffer();

	// render commands
	void cmd_submit_mesh(mesh_handle_t _mesh, material_handle_t _material, const glm::mat4 &_transform);
	void cmd_submit_entity(entity_handle_t _entity_handle);
	void cmd_flush();

	// performance stats
	void reset_perf_counters();
	void record_frame_time(float _dt_ms);
	void toggle_perf_overlay();
	void init_perf_graph();
	void show_notification(const std::string &_msg, float _duration_s=2.0f);
	void draw_notifications();
	void draw_perf_stats();
	void draw_frame_time_graph(float _x, float _y, float _w, float _h);

	// debug functions
private:
    void init_debug_rendering();
    void init_orienatation_obj(uint32_t _size);
public:
	void toggle_wireframe();
	void toggle_normals();
	void toggle_tangents();
	void toggle_bounding_boxes();
	void toggle_grid();

	void draw_debug_normals(mesh_handle_t _mesh_handle, const glm::mat4 &_transform);
	void draw_debug_tangents(mesh_handle_t _mesh_handle, const glm::mat4 &_transform);
	// if _entity is nullptr, AABBs of all entities in the entity library are drawn
	void draw_debug_bounding_box_entities(entity_t *_entity=nullptr);
	void draw_debug_bounding_boxes(const glm::vec3 &_min, const glm::vec3 &_max, const glm::mat4 &_transform);
	void draw_debug_bounding_boxes(entity_t *_entity=nullptr);
	void draw_debug_grid(float _y_level=0.0f);
	void draw_debug_orientation_obj();


public:
	// framebuffer storage
	GLuint m_stored_framebuffer = 0;
	framebuffer_t m_framebuffers[SYN_MAX_FRAMEBUFFERS];
	uint32_t m_frambuffer_count = 0;

	// main, final, framebuffer -- everything renders to this
	uint32_t m_scene_fbuffer_vao;
	shader_handle_t m_scene_fbuffer_shader_handle;
	framebuffer_handle_t m_scene_fbuffer_handle;
	framebuffer_t *m_scene_fbuffer;

	// ubos
	uniform_buffer_t m_material_ubo;
	uniform_buffer_t m_lighting_ubo;

	// lights
	light_internal_t m_lighting_state;

	// skybox
	skybox_t m_skybox;
	// IBL maps
	cubemap_handle_t m_irradiance_map;
	cubemap_handle_t m_prefilter_map;

	// render command
	render_command_t m_command_queue[SYN_MAX_RENDER_COMMANDS];
	uint32_t m_command_count;

	// performance stats
	perf_stats_t m_perf_stats; // .draw_calls_per_frame reset in root_window.prerender()

	// notifications
	struct {
	    std::string msg;
		float display_time = 0.0f;
		float duration = 2.0f;
	} m_notification;

	// debug (vaos and shader handles in debug_state_t)
	debug_state_t m_debug;
	bool m_debug_initialized = false;

	vertex_array_t m_orientation_obj_vao;
	shader_handle_t m_orientation_obj_shader_handle;
	uint32_t m_orientation_obj_size;

};



#endif // __RENDERER_H

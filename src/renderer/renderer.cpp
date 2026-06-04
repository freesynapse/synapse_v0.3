
#include <algorithm>
#include <string.h>
#include <string>

#include "renderer/renderer.h"
#include "renderer/mesh/mesh_generator.h"
#include "event/event_handler.h"
#include "renderer/material/material_types.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/shader/shader_library.h"
#include "utils/log.h"

#include "c_api.h"

// static event callback wrappers
static void __renderer_on_resize_callback(const event_t &_e) { renderer.on_resize(_e); }

//
void renderer_t::init()
{
    m_viewport = window.m_window_dim;

    // debug
    #ifdef DEBUG_OPENGL_API
    glDebugMessageCallback(openGLLogMessage, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    #endif
    
    // rendering and culling
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // GL_LINE and GL_POINT
    glLineWidth(1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1.0f);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    
    // textures
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SYN_ERROR("OpenGL error: %d.\n", error);
        glGetError();
    }
    
    // register function to receive viewport resize events
    events.register_callback(event_type_t::VIEWPORT_RESIZE,
                            __renderer_on_resize_callback);
    
    // initialize material uniform buffer
    init_material_ubo();

    // initialize lights
    init_lighting_ubo();

    // create skybox shader and mesh
    init_skybox();

    // This cannot be called if the skybox isnt complete (after set_skybox), since it is 
    // dependent on the m_skybox.cubmap_handle being vaild.
    // bake_irradiance_hdr();
    
    // initialize the render command queue
    memset(m_command_queue, 0,
            sizeof(render_command_t) * SYN_MAX_RENDER_COMMANDS);
    m_command_count = 0;

    // ui shader and quad for asset loading progress
    init_ui_quad();

    // DEBUG/DEV
    // 
    // setup vao and shader for rendering orientation object
    create_orienatation_obj(100);
    // perfomancce graph
    init_perf_graph();
    // debug geometry
    init_debug_rendering();
    
    
}

// 
void renderer_t::shutdown()
{
    shutdown_material_ubo();
    shutdown_lighting_ubo();
    
}

//
void renderer_t::on_resize(const event_t &_e)
{

    glm::ivec2 new_viewport = _e.as.viewport_resize.viewport;
    
    // set main viewport
    if (new_viewport.x > 0 && new_viewport.y > 0) {
        m_viewport = new_viewport;
        set_viewport(glm::ivec2(0, 0), new_viewport);
    
        if (m_scene_fbuffer) {
            m_scene_fbuffer->resize(m_viewport);
        }
    } else {
        SYN_WARNING("viewport not set : new viewport = [%d, %d]\n", new_viewport.x, new_viewport.y);
    }
}

//
void renderer_t::init_material_ubo()
{
    // must match GLSL layout (std140, location=)
    m_material_ubo.binding_point = 1;
    // m_material_ubo.size = sizeof(material_payload_t);
    m_material_ubo.size = SYN_MAX_MATERIAL_DATA_SIZE;
    
    glCreateBuffers(1, &m_material_ubo.opengl_id);
    
    // allocate immutable or dynamic device memory block spaces directly
    glNamedBufferData(m_material_ubo.opengl_id, m_material_ubo.size, nullptr, GL_DYNAMIC_DRAW);
    
    // bind the hardware asset space directly to the engine's global target index slot
    glBindBufferBase(GL_UNIFORM_BUFFER, m_material_ubo.binding_point, m_material_ubo.opengl_id);
    
}

//
void renderer_t::shutdown_material_ubo()
{
    if (m_material_ubo.opengl_id != 0) {
        glDeleteBuffers(1, &m_material_ubo.opengl_id);
        m_material_ubo.opengl_id = 0;
    }
}

// 
void renderer_t::init_lighting_ubo()
{
    //
    m_lighting_ubo.binding_point = 2;
    m_lighting_ubo.size = sizeof(light_internal_t);

    glCreateBuffers(1, &m_lighting_ubo.opengl_id);

    // allocate immutable or dynamic device memory block spaces directly
    glNamedBufferData(m_lighting_ubo.opengl_id, m_lighting_ubo.size, nullptr, GL_DYNAMIC_DRAW);
    
    // bind the hardware asset space directly to the engine's global target index slot
    glBindBufferBase(GL_UNIFORM_BUFFER, m_lighting_ubo.binding_point, m_lighting_ubo.opengl_id);

    memset(&m_lighting_state, 0, sizeof(light_internal_t));
    
}

// 
void renderer_t::shutdown_lighting_ubo()
{
    if (m_lighting_ubo.opengl_id != 0) {
        glDeleteBuffers(1, &m_lighting_ubo.opengl_id);
        m_lighting_ubo.opengl_id = 0;
    }
}

// 
void renderer_t::update_lighting_ubo()
{
    glNamedBufferSubData(m_lighting_ubo.opengl_id, 0, m_lighting_ubo.size, &m_lighting_state);
    
}

// 
void renderer_t::set_light(uint32_t _index, const light_t &_light)
{
    if (_index >= SYN_MAX_LIGHTS) {
        SYN_WARNING("SYN_MAX_LIGHTS exceeded.\n");
        return;
    }
    m_lighting_state.lights[_index] = _light;
    if (_index >= m_lighting_state.light_count) {
        m_lighting_state.light_count = _index + 1;
    }
    
}

// 
void renderer_t::init_skybox()
{
    m_skybox.cubemap_handle = { 0 };
    m_skybox.mesh_handle = { 0 };
    m_skybox.shader_handle = { 0 };
    
    mesh_handle_t mesh_handle = generate_skybox_cube();
    shader_handle_t shader_handle = shader_lib.load_from_file("skybox_shader", "../assets/shaders/skybox.glsl");

    if (!mesh_handle.is_valid() || !shader_handle.is_valid()) {
        SYN_WARNING("skybox initialization failed. mesh handle %d, shader handle %d.\n", mesh_handle.id, shader_handle.id);
        return;
    }

    m_skybox.mesh_handle = mesh_handle;
    m_skybox.shader_handle = shader_handle;
    
}

// 
void renderer_t::set_skybox(const cubemap_handle_t &_handle)
{
    if (!_handle.is_valid()) {
        SYN_WARNING("invalid cubemap handle (%d).\n", _handle.id);
        return;
    }

    m_skybox.cubemap_handle = _handle;

    // check that the shader and mesh are loaded too
    if (!m_skybox.mesh_handle.is_valid() || !m_skybox.shader_handle.is_valid()) {
        SYN_WARNING("skybox incomplete: mesh handle %d, shader handle %d.\n", m_skybox.mesh_handle.id, m_skybox.shader_handle.id);
    } else {
        m_skybox.is_active = true;
    }
    SYN_INFO("m_skybox.is_active = %s.\n", m_skybox.is_active ? "true" : "false");
    
}

// 
void renderer_t::render_skybox()
{
    if (!m_skybox.is_active) return;
    
    glDepthFunc(GL_LEQUAL);

    shader_t *sky_shader = shader_lib.get(m_skybox.shader_handle);
    sky_shader->enable();
    sky_shader->set_matrix_4fv("u_view", orbit_camera.get_view_matrix());
    sky_shader->set_matrix_4fv("u_projection", orbit_camera.get_projection_matrix());

    cubemap_internal_t *cubemap = cubemap_lib.get_cubemap(m_skybox.cubemap_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->opengl_id);
    sky_shader->set_uniform_1i("u_skybox_sampler", 0);

    const vertex_array_t *cube_vao = mesh_lib.get(m_skybox.mesh_handle);
    cube_vao->bind();
    glDrawElements(GL_TRIANGLES, cube_vao->get_index_count(), GL_UNSIGNED_INT, 0);
    m_perf_stats.draw_calls_per_frame++;

    cube_vao->unbind();
    glDepthFunc(GL_LESS);
    
}

// 
void renderer_t::bake_irradiance_hdr()
{
    time_step.start_timer();
    
    m_irradiance_map = cubemap_lib.create_empty(32, 32, GL_RGB16F);
    glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 capture_views[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +X
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // -X
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), // +Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // -Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +Z
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))  // -Z
    };

    // single use
    shader_t *conv_shader = shader_lib.get(shader_lib.load_from_file("irradiance_shader", "../assets/shaders/ibl_irradiance.glsl"));
    conv_shader->enable();
    conv_shader->set_matrix_4fv("u_projection", capture_proj);

    cubemap_internal_t *env_map = cubemap_lib.get_cubemap(m_skybox.cubemap_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map->opengl_id);
    conv_shader->set_uniform_1i("u_environment_map", 0);

    // render each face
    glViewport(0, 0, 32, 32);

    // temporary framebuffer
    GLuint capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    cubemap_internal_t *irr_map = cubemap_lib.get_cubemap(m_irradiance_map);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    for (uint32_t i = 0; i < 6; i++) {
        conv_shader->set_matrix_4fv("u_view", capture_views[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irr_map->opengl_id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const vertex_array_t *vao = mesh_lib.get(m_skybox.mesh_handle);
        vao->bind();
        glDrawElements(GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0);
    }

    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    reset_viewport();

}

// 
void renderer_t::bake_specular_hdr()
{
    m_prefilter_map = cubemap_lib.create_empty(128, 128, GL_RGB16F, true);
    glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 capture_views[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +X
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // -X
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), // +Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // -Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +Z
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))  // -Z
    };

    shader_t *spec_shader = shader_lib.get(shader_lib.load_from_file("prefilter_conv", "../assets/shaders/ibl_prefilter.glsl"));
    spec_shader->enable();
    spec_shader->set_uniform_1i("u_environment_map", 0);
    spec_shader->set_matrix_4fv("u_projection", capture_proj);

    cubemap_internal_t *env_map = cubemap_lib.get_cubemap(m_skybox.cubemap_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map->opengl_id);

    // temporary framebuffer
    GLuint capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    cubemap_internal_t *pref_map = cubemap_lib.get_cubemap(m_prefilter_map);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    uint32_t max_mip_levels = 5;
    for (uint32_t mip = 0; mip < max_mip_levels; mip++) {
        uint32_t mip_w = 128 >> mip;
        uint32_t mip_h = 128 >> mip;
        glViewport(0, 0, mip_w, mip_h);

        float roughness = (float)mip / (float)(max_mip_levels - 1);
        spec_shader->set_uniform_1f("u_roughness", roughness);

        for (uint32_t i = 0; i < 6; i++) {
            spec_shader->set_matrix_4fv("u_view", capture_views[i]);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pref_map->opengl_id, mip);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                SYN_ERROR("IBL bake framebuffer incomplete!\n");
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const vertex_array_t *vao = mesh_lib.get(m_skybox.mesh_handle);
            vao->bind();
            glDrawElements(GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0);
        }
    }

    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    reset_viewport();
    
}

// TODO : another pass at this at a later time perhaps, for now Im done.
cubemap_handle_t renderer_t::convert_equirect_to_cubemap(const texture_handle_t &_hdr_tex_handle)
{
    SYN_INFO("converting equirectangular HDR to cubemap...\n");

    // create an empty 512 x 512 HDR cubemap
    cubemap_handle_t handle = cubemap_lib.create_empty(512, 512, GL_RGB16F, true);
    cubemap_internal_t *cubemap = cubemap_lib.get_cubemap(handle);

    // linear minification filter while baking
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->opengl_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 capture_views[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +X
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // -X
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), // +Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // -Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +Z
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))  // -Z
    };

    //
    shader_t *convert_shader = shader_lib.get(shader_lib.load_from_file("hdr_convert_shader", "../assets/shaders/equirect_to_cube.glsl"));
    convert_shader->enable();
    convert_shader->set_matrix_4fv("u_projection", capture_proj);

    // bind hdr texture
    texture_internal_t *hdr_tex = tex_lib.get_texture(_hdr_tex_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_tex->opengl_id);
    convert_shader->set_uniform_1i("u_equirectangular_map", 0);

    // temp framebuffer
    // temporary framebuffer
    GLuint capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, 512, 512);

    for (uint32_t i = 0; i < 6; i++) {
        convert_shader->set_matrix_4fv("u_view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->opengl_id, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the cube mesh
        const vertex_array_t* vao = mesh_lib.get(m_skybox.mesh_handle);
        vao->bind();
        glDrawElements(GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->opengl_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return handle;
    
}

// 
framebuffer_handle_t renderer_t::create_framebuffer(const color_format_t& _format, 
				                                    const glm::ivec2& _size,
				                                    size_t _n_drawbuffers,
				                                    bool _use_depthbuffer,
				                                    const std::string& _name)
{
    if (m_frambuffer_count >= SYN_MAX_FRAMEBUFFERS) {
        SYN_WARNING("max framebuffers reached.\n");
        return { 0 };
    }

    uint32_t id = ++m_frambuffer_count;
    m_framebuffers[id - 1].create(_format, _size, _n_drawbuffers, _use_depthbuffer, _name);
    return { id };
    
}

// 
framebuffer_t *renderer_t::get_framebuffer(const framebuffer_handle_t &_handle)
{
    if (!_handle.is_active() || _handle.id > m_frambuffer_count) {
        return nullptr;
    }

    return &m_framebuffers[_handle.id - 1];
    
}

//
void renderer_t::create_scene_framebuffer()
{
    // initialize the render buffer
    //m_scene_fbuffer_sptr = MakeRef<framebuffer_t>();
    //m_scene_fbuffer_sptr->create(color_format_t::RGBA16F, glm::ivec2(0), 1, true, "scene_fbuffer");
    
    // SYN_INFO("created framebuffer '%s' (%dx%d).\n", m_scene_fbuffer_sptr->getName().c_str(), m_scene_fbuffer_sptr->getWidth(), m_scene_fbuffer_sptr->getHeight());

    m_scene_fbuffer_handle = create_framebuffer(color_format_t::RGBA16F, glm::ivec2(0), 1, true, "scene_fbuffer");
    m_scene_fbuffer = get_framebuffer(m_scene_fbuffer_handle);

    SYN_INFO("created framebuffer '%s' (%dx%d).\n", m_scene_fbuffer->getName().c_str(), 
        m_scene_fbuffer->getWidth(), m_scene_fbuffer->getHeight());
    
    // create the shader
    m_scene_fbuffer_shader_handle = shader_lib.load_from_file("scene_buffer_shader", 
                                                              "../assets/shaders/scene_fbuffer.glsl");
    // create the vertex array, no vertex data needed
    glCreateVertexArrays(1, &m_scene_fbuffer_vao);
    
}

//
void renderer_t::bind_scene_fbuffer()
{
    m_scene_fbuffer->bind();
}

//
void renderer_t::render_scene_fbuffer() 
{
    // unbind framebuffer, i.e. bind default buffer
    m_scene_fbuffer->unbind();

    // clear default buffer and disable depth test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // enable render buffer shader, set texture and bind color attachment of fbuffer
    shader_t *shader = shader_lib.get(m_scene_fbuffer_shader_handle);
    shader->enable();
    shader->set_uniform_1i("u_texture_sampler", 0);
    m_scene_fbuffer->bindTexture(0, 0);

    // bind vao and draw
    glBindVertexArray(m_scene_fbuffer_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    m_perf_stats.draw_calls_per_frame++;

    // reset
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

}

//
void renderer_t::cmd_submit_mesh(mesh_handle_t _mesh, material_handle_t _material, const glm::mat4 &_transform)
{
    if (m_command_count >= SYN_MAX_RENDER_COMMANDS) {
        SYN_WARNING("render command queue full, dropping command.\n");
        return;
    }
    render_command_t &cmd = m_command_queue[m_command_count];
    cmd.mesh = _mesh;
    cmd.material = _material;
    cmd.transform = _transform;
    m_command_count++;
}

// 
void renderer_t::cmd_submit_entity(entity_handle_t _entity_handle)
{
    entity_t *entity = entity_lib.get_entity(_entity_handle);
    cmd_submit_mesh(entity->mesh_handle, entity->material_handle, entity->transform);
}

//
void renderer_t::cmd_flush()
{
    if (m_command_count == 0) return;

    // 
    if (m_debug_state.show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    std::sort(m_command_queue, m_command_queue + m_command_count, 
        [](const render_command_t &a, const render_command_t &b) {
            return a.material.id < b.material.id;
        });

    glm::mat4 mat_vp = orbit_camera.get_view_projection_matrix();

    // track previously bound assets
    uint32_t current_active_shader_id = 0;
    shader_t *shader;
    
    //
    for (uint32_t i = 0; i < m_command_count; i++) {
        const render_command_t &cmd = m_command_queue[i];
        material_internal_t *mat = mat_lib.get_material(cmd.material);
        if (!mat) continue;

        // batching layer 1: only switch shader when needed
        shader = shader_lib.get(mat->shader_handle);
        if (shader && shader->get_id() != current_active_shader_id) {
            shader->enable();
            current_active_shader_id = shader->get_id();

            shader->set_matrix_4fv("u_view_projection", mat_vp);
            shader->set_uniform_3fv("u_view_pos", orbit_camera.get_position());
        }

        // batching layer 2: only update the material ubo when material changes
        if (i == 0 || m_command_queue[i - 1].material.id != cmd.material.id) {
            // glNamedBufferSubData(renderer.m_material_ubo.opengl_id, 0, sizeof(material_payload_t), &mat->payload);
            glNamedBufferSubData(renderer.m_material_ubo.opengl_id, 0, mat->data_size, mat->data);            

            // bind the different textures
            for (uint32_t slot = 0; slot < (uint32_t)texture_map_type_t::COUNT; slot++) {
                texture_handle_t handle = mat->textures[slot];
                if (handle.id != 0) {
                    texture_internal_t *tex = tex_lib.get_texture(handle);
                    if (tex && tex->opengl_id != 0) {
                        glBindTextureUnit(slot, tex->opengl_id);
                    } else {
                        glBindTextureUnit(slot, 0);
                    }
                }
            }
        }

        // bind irradiance map
        cubemap_internal_t *irradiance_map = cubemap_lib.get_cubemap(m_irradiance_map);
        if (irradiance_map) {
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map->opengl_id);
        }

        // bind prefilter map
        cubemap_internal_t *prefilter_map = cubemap_lib.get_cubemap(m_prefilter_map);
        if (prefilter_map) {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map->opengl_id);
        }
        
        // upload transforms (per object)
        glm::mat3 mat_normal = glm::transpose(glm::inverse(glm::mat3(cmd.transform)));
        shader->set_matrix_4fv("u_model", cmd.transform);
        shader->set_matrix_3fv("u_normal_matrix", mat_normal);

        // render mesh
        const vertex_array_t *vao = mesh_lib.get(cmd.mesh);
        vao->bind();
        glDrawElements(GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0);
        m_perf_stats.draw_calls_per_frame++;
        vao->unbind();
    }

    m_command_count = 0;

    if (m_debug_state.show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
}

// 
const glm::vec2 renderer_t::get_viewport_f() { return glm::vec2(m_viewport.x, m_viewport.y); }
float renderer_t::get_aspect_ratio() { return (float)m_viewport.x / (float)m_viewport.y; }

//
// API calls

// buffers
void renderer_t::clear_color_buffer() { glClear(GL_COLOR_BUFFER_BIT); }
void renderer_t::clear_depth_buffer() { glClear(GL_DEPTH_BUFFER_BIT); }
void renderer_t::clear(uint32_t _bitfield) { glClear(_bitfield); }
void renderer_t::set_clear_color(float _r, float _g, float _b, float _a)
{
    m_clear_color = glm::vec4(_r, _g, _b, _a);
    glClearColor(_r, _g, _b, _a);
}
void renderer_t::set_clear_color(const glm::vec4 &_color)
{
    m_clear_color = _color;
    glClearColor(_color.r, _color.g, _color.b, _color.a);
}

// viewport
void renderer_t::set_viewport(const glm::ivec2 &_position, const glm::ivec2 &_size)
{
    glViewport(_position.x, _position.y, _size.x, _size.y);
}

void renderer_t::reset_viewport() 
{
    glViewport(0, 0, m_viewport.x, m_viewport.y);
}

// blending equation
void renderer_t::set_blending_eq(GLenum _src_factor, GLenum _dest_factor) 
{
    glBlendFunc(_src_factor, _dest_factor);
}

void renderer_t::set_wireframe(bool _wireframe) 
{
    if (_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        return;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// 
void renderer_t::set_depth_testing(bool _depth_test) 
{
    if (_depth_test) {
        glEnable(GL_DEPTH_TEST);
        return;
    }
    glDisable(GL_DEPTH_TEST);
}

// 
void renderer_t::set_depth_mask(bool _depth_mask) 
{
    if (_depth_mask) {
        glDepthMask(GL_TRUE);
        return;
    }
    glDepthMask(GL_FALSE);
}

// 
void renderer_t::set_culling(bool _cull) 
{
    if (_cull) {
        glEnable(GL_CULL_FACE);
        return;
    }
    glDisable(GL_CULL_FACE);
}

// 
void renderer_t::set_blending(bool _blending) 
{
    m_is_blending = _blending;
    if (_blending) {
        glEnable(GL_BLEND);
        return;
    }
    glDisable(GL_BLEND);
}

// 
void renderer_t::set_GLenum(GLenum _gl_enum, bool _b) 
{
    if (_b) {
        glEnable(_gl_enum);
        return;
    }
    glDisable(_gl_enum);
}

//
void renderer_t::set_line_width(float _width) { glLineWidth(_width); }

// 
void renderer_t::init_ui_quad()
{
    // since glm::ortho, we need a different winding order
    glm::vec2 vertices[] = {
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 1.0f },
    };

    vertex_array_t vao;
    vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 }
    });

    // uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    vao.create(vertices, sizeof(vertices) / sizeof(glm::vec2));//, indices, 6);
    m_ui_quad_vao = vao;

    m_ui_shader_handle = shader_lib.load_from_file("ui_progress_shader", "../assets/shaders/ui_progress_shader.glsl");

    SYN_INFO("asset loader gui created.\n");
    
}

// 
void renderer_t::draw_rect(float _x, float _y, float _w, float _h, const glm::vec4 &_color)
{
    glm::mat4 projection = glm::ortho(0.0f, (float)m_viewport.x, (float)m_viewport.y, 0.0f);
    // translate and scale quad
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(_x, _y, 0.0f));
    model = glm::scale(model, glm::vec3(_w, _h, 1.0f));

    glDisable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);

    shader_t *shader = shader_lib.get(m_ui_shader_handle);
    shader->enable();
    shader->set_matrix_4fv("u_projection", projection);
    shader->set_matrix_4fv("u_model", model);
    shader->set_uniform_4fv("u_color", _color);

    m_ui_quad_vao.bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_perf_stats.draw_calls_per_frame++;
    m_ui_quad_vao.unbind();

    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
}

// 
void renderer_t::draw_rect_outline(float _x, float _y, 
                                   float _w, float _h, 
                                   float _thickness, 
                                   const glm::vec4 &_color,
                                   const glm::vec4 &_outline_color)
{
    glm::mat4 projection = glm::ortho(0.0f, (float)m_viewport.x, (float)m_viewport.y, 0.0f);
    // translate and scale quad
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(_x, _y, 0.0f));
    model = glm::scale(model, glm::vec3(_w, _h, 1.0f));

    glDisable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
    
    shader_t *shader = shader_lib.get(m_ui_shader_handle);
    shader->enable();
    shader->set_matrix_4fv("u_projection", projection);
    shader->set_matrix_4fv("u_model", model);

    m_ui_quad_vao.bind();

    shader->set_uniform_4fv("u_color", _color);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_perf_stats.draw_calls_per_frame++;

    shader->set_uniform_4fv("u_color", _outline_color);
    glLineWidth(_thickness);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    m_perf_stats.draw_calls_per_frame++;
    glLineWidth(1.0f);
    
    m_ui_quad_vao.unbind();
    
    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
}

// 
void renderer_t::reset_perf_counters()
{
    // called in syn_render_begin_3d
    m_perf_stats.draw_calls_per_frame = 0;
    
}

// 
void renderer_t::record_frame_time(float _dt_ms)
{
    float frame_time_lim = 1000.0f / time_step.fps_limit;
    m_perf_stats.frame_times[m_perf_stats.frame_time_idx] = _dt_ms;
    m_perf_stats.frame_time_idx = (m_perf_stats.frame_time_idx + 1) % SYN_PERF_GRAPH_SAMPLE_COUNT;

    // update max for auto-scaling
    m_perf_stats.max_frame_time = 0.0f;
    for (uint32_t i = 0; i < SYN_PERF_GRAPH_SAMPLE_COUNT; i++) {
        m_perf_stats.max_frame_time = std::max(m_perf_stats.max_frame_time, m_perf_stats.frame_times[i]);
    }

    // clamp min to 16.67 ms
    if (m_perf_stats.max_frame_time < frame_time_lim) {
        m_perf_stats.max_frame_time = frame_time_lim;
    }

}

// 
void renderer_t::toggle_perf_overlay()
{
    m_perf_stats.show_overlay = !m_perf_stats.show_overlay;
    m_perf_stats.show_graph = !m_perf_stats.show_graph;
}

// 
void renderer_t::init_perf_graph()
{
    if (m_perf_stats.graph_vao_initialized) return;

    m_perf_stats.graph_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 }
    });

    m_perf_stats.graph_vao.create_empty_vertices(SYN_PERF_GRAPH_SAMPLE_COUNT * 4 * 6 *sizeof(float));
    m_perf_stats.graph_vao.create_empty_indices(SYN_PERF_GRAPH_SAMPLE_COUNT * 6 * sizeof(uint32_t));

    m_perf_stats.graph_shader_handle = shader_lib.load_from_file("ui_perf_stats", "../assets/shaders/ui_perf_stats.glsl");
    
    m_perf_stats.graph_vao_initialized = true;
}

// 
void renderer_t::show_notification(const std::string &_msg, float _duration_s)
{
    m_notification.msg = _msg;
    m_notification.duration = _duration_s;
    m_notification.display_time = _duration_s;
}

// 
void renderer_t::draw_perf_stats()
{
    if (!m_perf_stats.show_overlay) return;

    // glDisable(GL_DEPTH_TEST);
    
    static float padding = 10.0f;
    static float line_height = font.get_font_height();

    static float x = padding;
    static float y = padding;
    
    // text
    float text_y = y;
    font.render_text(x, text_y += line_height, "FPS: %d (%.2f ms)", time_step.fps, time_step.dt * 1000.0f);
    font.render_text(x, text_y += line_height, "Draw Calls: %d", m_perf_stats.draw_calls_per_frame);

    if (m_perf_stats.show_graph) {
        static float w = 250.0f;
        static float h = 40.0f;
        text_y += padding;
        draw_frame_time_graph(x, text_y, w, h);
    }

    // show notification
    if (m_notification.display_time > 0.0f) {

        float msg_width = font.get_string_width("%s", m_notification.msg.c_str());

        float x = m_viewport.x * 0.5f - msg_width * 0.5f;
        float y = m_viewport.y - 200.0f;

        font.render_text(x, y, "%s", m_notification.msg.c_str());

        m_notification.display_time -= time_step.dt;
        
    }
    
    // glEnable(GL_DEPTH_TEST);
    
}

// 
void renderer_t::draw_frame_time_graph(float _x, float _y, float _w, float _h)
{
    if (!m_perf_stats.graph_vao_initialized) init_perf_graph();
    if (!m_perf_stats.show_graph) return;

    draw_rect_outline(_x, _y, _w, _h, 1.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f), glm::vec4(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)));

    // build the vertex data
    float vertices[4 * 6 * SYN_PERF_GRAPH_SAMPLE_COUNT];
    uint32_t indices[6 * SYN_PERF_GRAPH_SAMPLE_COUNT];

    uint32_t vert_count = 0;
    uint32_t vert_idx = 0;
    uint32_t idx_count = 0;

    float bar_width = _w / (float)SYN_PERF_GRAPH_SAMPLE_COUNT;
    float max_frame_time = (m_perf_stats.max_frame_time < 20.0f ? 20.0f : m_perf_stats.max_frame_time);

    for (uint32_t i = 0; i < SYN_PERF_GRAPH_SAMPLE_COUNT; i++) {
        uint32_t sample_idx = (m_perf_stats.frame_time_idx + i) % SYN_PERF_GRAPH_SAMPLE_COUNT;
        float frame_time = m_perf_stats.frame_times[sample_idx];

        if (frame_time == 0.0f) continue;

        float normalized = frame_time / max_frame_time;
        float bar_height = normalized * _h;
        float bar_x = _x + i * bar_width;
        float bar_y = _y + _h - bar_height;

        // color
        glm::vec4 color;
        if (frame_time <= 16.67f) {
            color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        } else if (frame_time <= 33.33f) {
            color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        } else {
            color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        uint32_t base_vert = vert_idx;
        
        float quad_verts[] = {
            // bottom-left (0)
             bar_x, bar_y + bar_height, color.r, color.g, color.b, color.a,
             // bottom-right (1)
             bar_x + bar_width, bar_y + bar_height, color.r, color.g, color.b, color.a,
             // top-left (2)
             bar_x, bar_y, color.r, color.g, color.b, color.a,
             // top-right (3)
             bar_x + bar_width, bar_y, color.r, color.g, color.b, color.a,            
        };

        memcpy(&vertices[vert_count], quad_verts, sizeof(quad_verts));
        vert_count += 24;

        indices[idx_count++] = base_vert + 0;
        indices[idx_count++] = base_vert + 1;
        indices[idx_count++] = base_vert + 2;
        indices[idx_count++] = base_vert + 1;
        indices[idx_count++] = base_vert + 3;
        indices[idx_count++] = base_vert + 2;

        vert_idx += 4;
    }

    if (idx_count == 0) return;

    // 
    glDisable(GL_DEPTH_TEST);
    shader_t *shader = shader_lib.get(m_perf_stats.graph_shader_handle);
    shader->enable();
    glm::mat4 projection = glm::ortho(0.0f, (float)m_viewport.x, (float)m_viewport.y, 0.0f);
    shader->set_matrix_4fv("u_projection", projection);
    shader->set_matrix_4fv("u_model", glm::mat4(1.0f));

    // update /vertices/indices and draw
    m_perf_stats.graph_vao.bind();
    m_perf_stats.graph_vao.update_vertices(vertices, sizeof(float) * vert_count);
    m_perf_stats.graph_vao.update_indices(indices, idx_count * sizeof(uint32_t));
    glDrawElements(GL_TRIANGLES, idx_count, GL_UNSIGNED_INT, 0);
    m_perf_stats.draw_calls_per_frame++;
    m_perf_stats.graph_vao.unbind();

    //
    font.render_text(_x + _w + 5.0f, _y, "%.1f ms", max_frame_time);
    font.render_text(_x + _w + 5.0f, _y + _h, "0 ms");
    
    glEnable(GL_DEPTH_TEST);
    
}

// 
void renderer_t::create_orienatation_obj(uint32_t _size)
{
    m_orientation_obj_size = _size;
    
    // create shader
    m_orientation_obj_shader_handle = shader_lib.load_from_file("orientation_obj_shader", 
                                                                "../assets/shaders/debug/orientation_obj.glsl");

    struct orientation_obj_v {
        glm::vec3 position;
        glm::vec3 color;
    };

    orientation_obj_v vertices[6] = {
        // x
        { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        // y
        { {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
        // z
        { {0.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 1.0f} },
        { {0.0f, 0.0f, 1.0f}, {0.2f, 0.2f, 1.0f} }    
    };

    m_orientation_obj_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT3 },
    });

    m_orientation_obj_vao.create(vertices, sizeof(vertices) / sizeof(orientation_obj_v), nullptr, 0);

    SYN_INFO("orientation visualizer created.\n");
    
}

// 
void renderer_t::render_orientation_obj()
{
    set_viewport({ 0, 0 }, { m_orientation_obj_size, m_orientation_obj_size });

    // render on top
    clear_depth_buffer();

    // 
    glm::mat4 cam_rot = glm::mat4(glm::mat3(orbit_camera.get_view_matrix()));
    static glm::mat4 ortho_proj = glm::ortho(-1.1f, 1.1f, -1.1f, 1.1f, -1.1f, 1.1f);
    glm::mat4 mvp = ortho_proj * cam_rot;

    shader_t *shader = shader_lib.get(m_orientation_obj_shader_handle);
    shader->enable();
    shader->set_matrix_4fv("u_mvp", mvp);

    m_orientation_obj_vao.bind();
    set_line_width(3.0f);
    glDrawArrays(GL_LINES, 0, 6);
    m_perf_stats.draw_calls_per_frame++;
    set_line_width(1.0f);

    shader->disable();

    reset_viewport();
    
}

// 
void renderer_t::init_debug_rendering()
{
    m_debug_state.debug_normal_shader_handle = shader_lib.load_from_file("debug_normal_shader", 
        "../assets/shaders/debug/debug_mesh_normals.glsl");

    m_debug_state_initialized = true;
}

// 
void renderer_t::toggle_wireframe() { m_debug_state.show_wireframe = !m_debug_state.show_wireframe;  SYN_INFO("show_wireframe = %d.\n", m_debug_state.show_wireframe);  }
void renderer_t::toggle_normals()   { m_debug_state.show_normals   = !m_debug_state.show_normals;    SYN_INFO("show_normals = %d.\n", m_debug_state.show_normals);      }
void renderer_t::toggle_tangents()  { m_debug_state.show_tangents  = !m_debug_state.show_tangents;   SYN_INFO("show_tangents = %d.\n", m_debug_state.show_tangents);    }
void renderer_t::toggle_grid()      { m_debug_state.show_grid      = !m_debug_state.show_grid;       SYN_INFO("show_grid = %d.\n", m_debug_state.show_grid);            }

void renderer_t::draw_debug_normals(mesh_handle_t _mesh_handle, const glm::mat4 &_transform)
{
    if (!m_debug_state.show_normals && !m_debug_state.show_tangents) return;
    if (!m_debug_state_initialized) return;

    const vertex_array_t *vao = mesh_lib.get(_mesh_handle);
    if (!vao) return;

    shader_t *shader = shader_lib.get(m_debug_state.debug_normal_shader_handle);
    shader->enable();

    glm::mat4 vp = orbit_camera.get_view_projection_matrix();
    glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(_transform)));

    shader->set_matrix_4fv("u_model", _transform);
    shader->set_matrix_4fv("u_view_projection", vp);
    shader->set_matrix_3fv("u_normal_matrix", normal_matrix);
    shader->set_uniform_1f("u_normal_length", m_debug_state.normal_length);
    shader->set_uniform_1i("u_show_tangents", m_debug_state.show_tangents ? 1 : 0);

    vao->bind();
    glDrawElements(GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0);
    vao->unbind();
    
}

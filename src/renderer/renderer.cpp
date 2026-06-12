
#include <algorithm>
#include <string.h>
#include <string>

#include "event/event_handler.h"
#include "renderer/material/material_types.h"
#include "renderer/mesh/mesh_generator.h"
#include "renderer/mesh/mesh_types.h"
#include "renderer/renderer.h"
#include "renderer/shader/shader_library.h"
#include "utils/log.h"

#include "c_api.h"

// static event callback wrappers
static void __renderer_on_resize_callback(const event_t &_e) { renderer.on_resize(_e); }

//
void renderer_t::init() 
{
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

    // enable ability to split batched GL_LINE_STRIP element draw calls
    // [ 0, 1, 2, 3,  0xFFFFFFFF,  4, 5, 6,  0xFFFFFFFF,  7, 8, 9, 10 ]
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);
    
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
    events.register_callback(event_type_t::VIEWPORT_RESIZE, __renderer_on_resize_callback);
    
    // initialize material uniform buffer
    init_material_ubo();
    
    // initialize lights
    init_lighting_ubo();
    
    // create skybox shader and mesh
    init_skybox();
    
    // bake_irradiance_hdr(); <- cannot be called unless m_skybox is intialized
    
    // initialize the render command queue
    memset(m_command_queue, 0, sizeof(render_command_t) * SYN_MAX_RENDER_COMMANDS);
    m_command_count = 0;
    
    // DEBUG/DEV
    
    // debug geometry
    init_debug_rendering();
    
}

//
void renderer_t::shutdown()
{
    shutdown_material_ubo();
    shutdown_lighting_ubo();
    
    // cleanup debug
    if (m_debug.grid_vao_id != 0) {
        glDeleteVertexArrays(1, &m_debug.grid_vao_id);
    }
}

//
void renderer_t::on_resize(const event_t &_e)
{
    glm::ivec2 new_viewport = _e.as.viewport_resize.viewport;

    if (new_viewport.x > 0 && new_viewport.y > 0 && m_scene_fbuffer_handle.id > 0) {
        // this should work, since the api is initialized before the renderer,
        // meaning that api.onresize have already updated the api.m_viewport when
        // this gets called.
        framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);
        fbo->resize(api.m_viewport);
    }
}

//
void renderer_t::init_material_ubo()
{
    // must match GLSL layout (std140, location=)
    m_material_ubo.binding_point = 1;
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
        SYN_WARNING("skybox initialization failed. mesh handle %d, shader handle %d.\n",
                    mesh_handle.id, shader_handle.id);
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
        SYN_WARNING("skybox incomplete: mesh handle %d, shader handle %d.\n",
                    m_skybox.mesh_handle.id, m_skybox.shader_handle.id);
    } else {
        m_skybox.is_active = true;
    }
  
    SYN_INFO("m_skybox.is_active = %s.\n", m_skybox.is_active ? "true" : "false");
    
}

//
void renderer_t::render_skybox()
{
    if (!m_skybox.is_active)
        return;
    
    glDepthFunc(GL_LEQUAL);
    
    shader_t *sky_shader = shader_lib.get_shader(m_skybox.shader_handle);
    sky_shader->enable();
    sky_shader->set_matrix_4fv("u_view", cam.get_view_matrix());
    sky_shader->set_matrix_4fv("u_projection", cam.get_projection_matrix());
    
    cubemap_internal_t *cubemap = cubemap_lib.get_cubemap(m_skybox.cubemap_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->opengl_id);
    sky_shader->set_uniform_1i("u_skybox_sampler", 0);
    
    mesh_internal_t *mesh = mesh_lib.get_mesh(m_skybox.mesh_handle);
    if (!mesh) return;
    
    mesh->vao.bind();
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    mesh->vao.unbind();
    
    m_perf_stats.draw_calls_per_frame++;
    
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
    shader_t *conv_shader = shader_lib.get_shader(
        shader_lib.load_from_file("irradiance_shader", "../assets/shaders/ibl_irradiance.glsl")
    );
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
    
        mesh_internal_t *mesh = mesh_lib.get_mesh(m_skybox.mesh_handle);
        if (!mesh) return;
    
        mesh->vao.bind();
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        mesh->vao.unbind();
    }
    
    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    api.reset_viewport();
  
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
    
    shader_t *spec_shader = shader_lib.get_shader(
        shader_lib.load_from_file("prefilter_conv", "../assets/shaders/ibl_prefilter.glsl")
    );
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
        
            mesh_internal_t *mesh = mesh_lib.get_mesh(m_skybox.mesh_handle);
            if (!mesh) return;
        
            mesh->vao.bind();
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
            mesh->vao.unbind();
        }
    }
    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    api.reset_viewport();
  
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
    
    glm::mat4 capture_proj =
        glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 capture_views[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +X
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // -X
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), // +Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // -Y
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +Z
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)) // -Z
    };
    
    //
    shader_t *convert_shader = shader_lib.get_shader(shader_lib.load_from_file(
        "hdr_convert_shader", "../assets/shaders/equirect_to_cube.glsl"));
    convert_shader->enable();
    convert_shader->set_matrix_4fv("u_projection", capture_proj);
    
    // bind hdr texture
    texture_internal_t *hdr_tex = tex_lib.get_texture(_hdr_tex_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_tex->opengl_id);
    convert_shader->set_uniform_1i("u_equirectangular_map", 0);
    
    // temp framebuffer
    GLuint capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glViewport(0, 0, 512, 512);
    
    for (uint32_t i = 0; i < 6; i++) {
        convert_shader->set_matrix_4fv("u_view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               cubemap->opengl_id, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    
        // Render the cube mesh
        mesh_internal_t *mesh = mesh_lib.get_mesh(m_skybox.mesh_handle);
        if (!mesh) return { 0 };
    
        mesh->vao.bind();
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        mesh->vao.unbind();
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->opengl_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    return handle;

}


//
// void renderer_t::create_scene_framebuffer() 
// {
//     m_scene_fbuffer_handle = api.fbo_handler.create_framebuffer(color_format_t::RGBA16F, glm::ivec2(0), 1, true, "scene_fbuffer");
//     framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);

//     SYN_INFO("created framebuffer '%s' (%dx%d).\n", fbo->get_name().c_str(), fbo->get_width(), fbo->get_height());
    
//     // create the shader
//     m_scene_fbuffer_shader_handle = shader_lib.load_from_file("scene_buffer_shader", 
//         "../assets/shaders/scene_fbuffer.glsl");

//     // create the vertex array, no vertex data needed
//     glCreateVertexArrays(1, &m_scene_fbuffer_vao);
    
// }

//
void renderer_t::bind_scene_fbuffer()
{
    if (m_scene_fbuffer_handle.id == 0) {
        SYN_WARNING("invalid m_scene_fbuffer_handle: was the 3d scene bufffer created?\n"); 
        return;
    }
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);
    fbo->bind(); 

}

// 
void renderer_t::unbind_scene_fbuffer()
{
    // assumes that the buffer was bound, warning message already logged, so 
    // here we just use the !fbo guard (handle would be 0 -> nullptr).
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);
    if (fbo) fbo->unbind(); 

}

//
void renderer_t::render_scene_fbuffer()
{
    // unbind framebuffer, i.e. bind default buffer
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);
    if (!fbo) {
        
        return;
    }
    
    fbo->unbind(); 
    
    // clear default buffer and disable depth test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // enable render buffer shader, set texture and bind color attachment of
    // fbuffer
    shader_t *shader = shader_lib.get_shader(m_scene_fbuffer_shader_handle);
    shader->enable();
    fbo->bind_texture(0, 0);
    
    // bind vao and draw
    glBindVertexArray(m_scene_fbuffer_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    m_perf_stats.draw_calls_per_frame++;
    
    // reset
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    
}

//
void renderer_t::cmd_submit_mesh(mesh_handle_t _mesh,
                                 material_handle_t _material,
                                 const glm::mat4 &_transform)
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
    cmd_submit_mesh(entity->mesh_handle, entity->material_handle,
                    entity->transform);
}

//
void renderer_t::cmd_flush()
{
    if (m_command_count == 0)
        return;
    
    //
    if (m_debug.show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    std::sort(m_command_queue, m_command_queue + m_command_count,
                [](const render_command_t &a, const render_command_t &b) {
                return a.material.id < b.material.id;
                });
    
    glm::mat4 mat_vp = cam.get_view_projection_matrix();
    
    // track previously bound assets
    uint32_t current_active_shader_id = 0;
    shader_t *shader;
    
    //
    for (uint32_t i = 0; i < m_command_count; i++) {
        const render_command_t &cmd = m_command_queue[i];
        material_internal_t *mat = mat_lib.get_material(cmd.material);
        if (!mat) continue;
    
        // batching layer 1: only switch shader when needed
        shader = shader_lib.get_shader(mat->shader_handle);
        if (shader && shader->get_id() != current_active_shader_id) {
            shader->enable();
            current_active_shader_id = shader->get_id();
        
            shader->set_matrix_4fv("u_view_projection", mat_vp);
            shader->set_uniform_3fv("u_view_pos", cam.get_position());
        }
    
        // batching layer 2: only update the material ubo when material changes
        if (i == 0 || m_command_queue[i - 1].material.id != cmd.material.id) {
            // glNamedBufferSubData(renderer.m_material_ubo.opengl_id, 0,
            // sizeof(material_payload_t), &mat->payload);
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
        mesh_internal_t *mesh = mesh_lib.get_mesh(cmd.mesh);
        if (!mesh) return;
    
        mesh->vao.bind();
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        m_perf_stats.draw_calls_per_frame++;
        mesh->vao.unbind();
    }
    
    m_command_count = 0;
    
    if (m_debug.show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

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
    
    float padding = 10.0f;
    float line_height = font.get_font_height();
    
    float x = padding;
    float y = padding;
    
    // text
    float text_y = y + line_height;
    font.render_text(x, text_y += line_height, "FPS: %d (%.2f ms)", time_step.fps, time_step.dt * 1000.0f);
    font.render_text(x, text_y += line_height, "Draw Calls: %d", m_perf_stats.draw_calls_per_frame);
    
    if (m_perf_stats.show_graph) {
        static float w = 250.0f;
        static float h = 40.0f;
        text_y += padding;
        draw_frame_time_graph(x, text_y, w, h);
    }
    
}

// 
void renderer_t::draw_notifications()
{
    if (m_notification.display_time > 0.0f) {
        float alpha = glm::clamp(m_notification.display_time / m_notification.duration, 0.0f, 1.0f);
        float msg_width = font.get_string_width("%s", m_notification.msg.c_str());
    
        glm::vec2 vp = api.get_scene_fviewport();
        float x = vp.x  * 0.5f - msg_width * 0.5f;
        float y = vp.y - 200.0f;
    
        glm::vec4 prev_color = font.get_color();
        glm::vec4 color = prev_color;
        color.a = alpha;
        font.set_color(color);
        font.render_text(x, y, "%s", m_notification.msg.c_str());
        font.set_color(prev_color);
    
        m_notification.display_time -= time_step.dt;
    }
    
}

//
void renderer_t::draw_frame_time_graph(float _x, float _y, float _w, float _h)
{
    if (!m_perf_stats.show_graph) return;

    float bar_width = _w / (float)SYN_PERF_GRAPH_SAMPLE_COUNT;
    float max_frame_time = (m_perf_stats.max_frame_time < 20.0f ? 20.0f : m_perf_stats.max_frame_time);

    renderer_2d.batch.begin_batch();
    
    // background and outline
    renderer_2d.batch.add_quad({ _x, _y }, { _w, _h }, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f), -1.0f);
    ui_render_vertex_t outline[] = {
        ui_render_vertex_t({ _x,      _y      }, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), -0.9f),
        ui_render_vertex_t({ _x + _w, _y      }, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), -0.9f),
        ui_render_vertex_t({ _x + _w, _y + _h }, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), -0.9f),
        ui_render_vertex_t({ _x,      _y + _h }, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), -0.9f),
        ui_render_vertex_t({ _x,      _y      }, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), -0.9f),
    };
    renderer_2d.batch.add_line_strip(outline, 5);

    // bars
    for (uint32_t i = 0; i < SYN_PERF_GRAPH_SAMPLE_COUNT; i++) {
        uint32_t sample_idx = (m_perf_stats.frame_time_idx + i) % SYN_PERF_GRAPH_SAMPLE_COUNT;
        float frame_time = m_perf_stats.frame_times[sample_idx];
        if (frame_time == 0.0f) continue;

        float normalized  = frame_time / max_frame_time;
        float bar_height  = normalized * _h;
        float bar_x       = _x + i * bar_width;
        float bar_y       = _y + _h - bar_height;

        glm::vec4 color;
        if      (frame_time <= 16.67f) color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        else if (frame_time <= 33.33f) color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        else                           color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        renderer_2d.batch.add_quad({ bar_x, bar_y }, { bar_width, bar_height }, color, -0.5f);
    }

    renderer_2d.batch.end_batch();

    // labels
    font.render_text(_x + _w + 5.0f, _y, "%.1f ms", max_frame_time);
    font.render_text(_x + _w + 5.0f, _y + _h, "0 ms");
    
}

//
void renderer_t::init_debug_rendering() 
{
    m_debug.normal_shader_handle = shader_lib.load_from_file("debug_normal_shader", 
        "../assets/shaders/debug/debug_mesh_normals.glsl");
    
    m_debug.line_shader_handle = shader_lib.load_from_file("debug_line_shader", 
        "../assets/shaders/debug/debug_lines.glsl");
    
    // shader contains posistion (vec3) and color (vec4)
    size_t max_lines = 10000;
    size_t buffer_size = max_lines * 2 * (sizeof(glm::vec3) + sizeof(glm::vec4));

    m_debug.line_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 }
    });
    m_debug.line_vao.create_empty_vertices(buffer_size);
    
    // grid
    m_debug.grid_shader_handle = shader_lib.load_from_file("debug_grid_shader", 
        "../assets/shaders/debug/debug_grid.glsl");
    
    // create a custom vao, since no vbo is needed
    glGenVertexArrays(1, &m_debug.grid_vao_id);
    
    //
    m_debug_initialized = true;

    init_orienatation_obj(100);
    
}

//
void renderer_t::init_orienatation_obj(uint32_t _size)
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
void renderer_t::toggle_wireframe()      { m_debug.show_wireframe      = !m_debug.show_wireframe;       }
void renderer_t::toggle_normals()        { m_debug.show_normals        = !m_debug.show_normals;         }
void renderer_t::toggle_tangents()       { m_debug.show_tangents       = !m_debug.show_tangents;        }
void renderer_t::toggle_bounding_boxes() { m_debug.show_bounding_boxes = !m_debug.show_bounding_boxes;  }
void renderer_t::toggle_grid()           { m_debug.show_grid           = !m_debug.show_grid;            }
//
void renderer_t::render_debug_normals(mesh_handle_t _mesh_handle, const glm::mat4 &_transform)
{
    if (!m_debug.show_normals && !m_debug.show_tangents) return;
    if (!m_debug_initialized) return;
    
    mesh_internal_t *mesh = mesh_lib.get_mesh(_mesh_handle);
    if (!mesh) return;
    
    shader_t *shader = shader_lib.get_shader(m_debug.normal_shader_handle);
    if (!shader) return;
    
    shader->enable();
    
    glm::mat4 vp = cam.get_view_projection_matrix();
    glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(_transform)));
    
    shader->set_matrix_4fv("u_model", _transform);
    shader->set_matrix_4fv("u_view_projection", vp);
    shader->set_matrix_3fv("u_normal_matrix", normal_matrix);
    shader->set_uniform_1f("u_normal_length", m_debug.normal_length);
    shader->set_uniform_1i("u_show_tangents", m_debug.show_tangents ? 1 : 0);
    
    mesh->vao.bind();
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    mesh->vao.unbind();
    
}

//
void renderer_t::render_debug_bounding_box_entities(entity_t *_entity)
{
    if (!_entity) {
        for (uint32_t i = 0; i < entity_lib.m_active_count; i++) {
            entity_t *e = &entity_lib.m_pool[i];
            mesh_internal_t *mesh = mesh_lib.get_mesh(e->mesh_handle);
            if (mesh) {
                renderer.render_debug_bounding_boxes(mesh->aabb_min, mesh->aabb_max, e->transform);
            }
        }
    } else {
        mesh_internal_t *mesh = mesh_lib.get_mesh(_entity->mesh_handle);
        if (mesh) {
            renderer.render_debug_bounding_boxes(mesh->aabb_min, mesh->aabb_max, _entity->transform);
        }
    }
}

//
void renderer_t::render_debug_bounding_boxes(const glm::vec3 &_min,
                                           const glm::vec3 &_max,
                                           const glm::mat4 &_transform)
{
    if (!m_debug.show_bounding_boxes) return;
    if (!m_debug_initialized) init_debug_rendering();
    
    glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f);
    
    // local coordinate corners
    glm::vec3 corners[8] = {
        glm::vec3(_min.x, _min.y, _min.z), // 0: bottom-left-back
        glm::vec3(_max.x, _min.y, _min.z), // 1: bottom-right-back
        glm::vec3(_max.x, _max.y, _min.z), // 2: top-right-back
        glm::vec3(_min.x, _max.y, _min.z), // 3: top-left-back
        glm::vec3(_min.x, _min.y, _max.z), // 4: bottom-left-front
        glm::vec3(_max.x, _min.y, _max.z), // 5: bottom-right-front
        glm::vec3(_max.x, _max.y, _max.z), // 6: top-right-front
        glm::vec3(_min.x, _max.y, _max.z), // 7: top-left-front
    };
    
    for (uint32_t i = 0; i < 8; i++) {
        glm::vec4 world_pos = _transform * glm::vec4(corners[i], 1.0f);
        corners[i] = glm::vec3(world_pos);
    }
    
    // vertices
    std::vector<float> lines;
    
    auto add_line = [&](int a, int b) {
        // vertex 0
        lines.push_back(corners[a].x); lines.push_back(corners[a].y); lines.push_back(corners[a].z);
        lines.push_back(color.r); lines.push_back(color.g); lines.push_back(color.b); lines.push_back(color.a);
        // vertex 1
        lines.push_back(corners[b].x); lines.push_back(corners[b].y); lines.push_back(corners[b].z);
        lines.push_back(color.r); lines.push_back(color.g); lines.push_back(color.b); lines.push_back(color.a);
    };
    
    add_line(0, 1); add_line(1, 2); add_line(2, 3); add_line(3, 0);
    add_line(4, 5); add_line(5, 6); add_line(6, 7); add_line(7, 4);
    add_line(0, 4); add_line(1, 5); add_line(2, 6); add_line(3, 7);
    
    m_debug.line_vao.bind();
    m_debug.line_vao.update_vertices((void *)&lines[0], lines.size() * sizeof(float));
    
    shader_t *shader = shader_lib.get_shader(m_debug.line_shader_handle);
    if (!shader) return;
    
    shader->enable();
    shader->set_matrix_4fv("u_view_projection", cam.get_view_projection_matrix());
    
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, lines.size() / 7);
    glLineWidth(1.0f);
    
    m_debug.line_vao.unbind();
    
    m_perf_stats.draw_calls_per_frame++;
    
}

//
void renderer_t::render_debug_grid(float _y_level)
{
    if (!m_debug.show_grid) return;
    
    shader_t *shader = shader_lib.get_shader(m_debug.grid_shader_handle);
    if (!shader) return;
    
    glBindVertexArray(m_debug.grid_vao_id);
    
    shader->enable();
    
    glm::mat4 vp = cam.get_view_projection_matrix();
    shader->set_matrix_4fv("u_inv_view_projection", glm::inverse(vp));
    shader->set_matrix_4fv("u_view_projection", vp);
    shader->set_uniform_1f("u_near", cam.get_z_near());
    shader->set_uniform_1f("u_far", cam.get_z_far());
    shader->set_uniform_1f("u_grid_y", _y_level);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    m_perf_stats.draw_calls_per_frame++;
    
}

//
void renderer_t::render_debug_orientation_obj()
{
    api.set_viewport({0, 0}, { m_orientation_obj_size, m_orientation_obj_size });
    
    // render on top
    api.clear_depth_buffer();
    
    //
    glm::mat4 cam_rot = glm::mat4(glm::mat3(cam.get_view_matrix()));
    static glm::mat4 ortho_proj = glm::ortho(-1.1f, 1.1f, -1.1f, 1.1f, -1.1f, 1.1f);
    glm::mat4 mvp = ortho_proj * cam_rot;
    
    shader_t *shader = shader_lib.get_shader(m_orientation_obj_shader_handle);
    shader->enable();
    shader->set_matrix_4fv("u_mvp", mvp);
    
    m_orientation_obj_vao.bind();
    api.set_line_width(3.0f);
    glDrawArrays(GL_LINES, 0, 6);
    m_perf_stats.draw_calls_per_frame++;
    api.set_line_width(1.0f);
    
    shader->disable();
    
    api.reset_viewport();

}


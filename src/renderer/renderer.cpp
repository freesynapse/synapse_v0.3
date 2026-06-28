
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
#include "utils/math_utils.h"

#include "c_api.h"

// static event callback wrappers
static void __renderer_on_resize_callback(const event_t &_e) { renderer.on_resize(_e); }

//
void renderer_t::init() 
{
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

    // setup shadow map -- off by default
    init_shadow_map();
    
    // bake_irradiance_hdr(); <- cannot be called unless m_skybox is intialized
    
    // initialize the render command queue
    memset(m_command_queue, 0, sizeof(render_command_t) * SYN_MAX_RENDER_COMMANDS);
    m_command_count = 0;
    
    // DEBUG/DEV
    
    // debug geometry
    init_debug_rendering();

    // orientation object
    init_orienatation_obj(100);

    // ui elements
    init_ui_rendering();    // viewport entity manipulation ui

}

//
void renderer_t::shutdown()
{
    release_material_ubo();
    release_lighting_ubo();
    release_shadow_map();
    
    // cleanup debug
    if (debug.grid_vao_id != 0) {
        glDeleteVertexArrays(1, &debug.grid_vao_id);
    }

    shutdown_ui_rendering();
    
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
void renderer_t::release_material_ubo()
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
void renderer_t::release_lighting_ubo()
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
    m_skybox.mesh_handle    = { 0 };
    m_skybox.shader_handle  = { 0 };
    
    mesh_handle_t   mesh_handle   = mesh_generator.create_skybox_cube_mesh();
    shader_handle_t shader_handle = shader_lib.load_from_file("skybox_shader", "../assets/shaders/skybox.glsl");
    
    if (!mesh_handle.is_valid() || !shader_handle.is_valid()) {
        SYN_WARNING("skybox initialization failed. mesh handle %d, shader handle %d.\n",
                    mesh_handle.id, shader_handle.id);
        return;
    }
    
    m_skybox.mesh_handle   = mesh_handle;
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
  
}

//
void renderer_t::render_skybox()
{
    if (!m_skybox.is_active) return;

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
glm::vec4 renderer_t::skybox_find_sun_direction()
{
    if (!m_skybox.is_active) return glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);

    cubemap_internal_t *cm = cubemap_lib.get_cubemap(m_skybox.cubemap_handle);
    if (!cm) return glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);

    std::vector<float> pixels(cm->width * cm->height * 3);

    glm::vec3 brightest_dir = { 0.0f, 1.0f, 0.0f };
    float brightest = 0.0f;

    const glm::vec3 face_forward[6] = {{1,0,0},{-1,0,0 },{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const glm::vec3 face_right[6]   = {{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const glm::vec3 face_up[6]      = {{0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0}};

    glBindTexture(GL_TEXTURE_CUBE_MAP, cm->opengl_id);

    for (int face = 0; face < 6; face++) {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, pixels.data());
        for (uint32_t y = 0; y < cm->height; y++) {
            for (uint32_t x = 0; x < cm->width; x++) {
                int idx = (y * cm->width + x) * 3;
                float r = pixels[idx + 0];
                float g = pixels[idx + 1];
                float b = pixels[idx + 2];

                // luminance since HDR values can be >> 1
                float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                if (lum > brightest) {
                    brightest = lum;
                    // convert texel to normalized [-1..1] face coordinates
                    float u = (x + 0.5f) / cm->width  * 2.0f - 1.0f;
                    float v = (y + 0.5f) / cm->height * 2.0f - 1.0f;
                    // reconstruct world-space directions for this texel
                    glm::vec3 dir = glm::normalize(face_forward[face] + face_right[face] * u + face_up[face] * v);
                    brightest_dir = dir;
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -glm::vec4(brightest_dir, 0.0f);
    
}

// 
void renderer_t::bake_ibl()
{
    bake_irradiance_hdr();
    bake_specular_hdr();
    
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
cubemap_handle_t renderer_t::convert_equirect_to_cubemap(const texture_handle_t &_hdr_tex_handle, uint32_t _tex_size)
{
    SYN_INFO("converting equirectangular HDR to cubemap...\n");
    
    // create an empty HDR cubemap
    uint32_t tex_size = _tex_size;
    cubemap_handle_t handle = cubemap_lib.create_empty(tex_size, tex_size, GL_RGB16F, true);
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
    
    glViewport(0, 0, tex_size, tex_size);
    
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
void renderer_t::init_shadow_map()
{
    m_shadow_shader_handle = shader_lib.load_from_file("shadow_map_shader", "../assets/shaders/shadow_map.glsl");
    if (!m_shadow_shader_handle.is_valid()) {
        SYN_WARNING("shadow map shader failed to load.\n");
        return;
    }

    // depth texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_shadow_map.depth_id);
    glTextureStorage2D(m_shadow_map.depth_id, 1, GL_DEPTH_COMPONENT32F, SYN_SHADOW_MAP_SIZE, SYN_SHADOW_MAP_SIZE);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(m_shadow_map.depth_id, GL_TEXTURE_BORDER_COLOR, border_color);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTextureParameteri(m_shadow_map.depth_id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // depth-only fbo
    glCreateFramebuffers(1, &m_shadow_map.fbo_id);
    glNamedFramebufferTexture(m_shadow_map.fbo_id, GL_DEPTH_ATTACHMENT, m_shadow_map.depth_id, 0);
    glNamedFramebufferDrawBuffer(m_shadow_map.fbo_id, GL_NONE);
    glNamedFramebufferReadBuffer(m_shadow_map.fbo_id, GL_NONE);

    if (glCheckNamedFramebufferStatus(m_shadow_map.fbo_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SYN_ERROR("shadow map framebuffer incomplete.\n");
        return;
    }

    SYN_INFO("shadow map initialized (%dx%d).\n", SYN_SHADOW_MAP_SIZE, SYN_SHADOW_MAP_SIZE);
    
}

// 
void renderer_t::release_shadow_map()
{
    if (m_shadow_map.fbo_id != 0) {
        glDeleteFramebuffers(1, &m_shadow_map.fbo_id);
        m_shadow_map.fbo_id = 0;
    }

    if (m_shadow_map.depth_id != 0) {
        glDeleteTextures(1, &m_shadow_map.depth_id);
        m_shadow_map.depth_id = 0;
    }
    
}

// 
void renderer_t::render_shadow_pass()
{
    if (!m_shadow_map.is_active) return;
    if (m_command_count == 0)    return;

    shader_t *shader = shader_lib.get_shader(m_shadow_shader_handle);
    if (!shader) return;

    // build light-space matrix from lights[0] (directional ligth)
    glm::vec3 light_dir = glm::normalize(glm::vec3(m_lighting_state.lights[0].direction));

    // TODO : fix this: at far distances the shadows disappear
    float cam_dist = m_shadow_map.ortho_size;

    //glm::vec3 light_pos = -light_dir * cam_dist;
    glm::vec3 scene_center = cam.get_position();
    scene_center.y = 0.0f;
    glm::vec3 light_pos = scene_center - light_dir * cam_dist;
    
    // glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 light_view = glm::lookAt(light_pos, scene_center, glm::vec3(0.0f, 1.0f, 0.0f));

    // setup scene-dependent projection (base viewing frustum based on entities)
    glm::vec3 ls_min( FLT_MAX);
    glm::vec3 ls_max(-FLT_MAX);
    
    for (uint32_t i = 0; i < m_command_count; i++) {
        const render_command_t &cmd = m_command_queue[i];
        mesh_internal_t *mesh = mesh_lib.get_mesh(cmd.mesh);
        if (!mesh) continue;

        // transform AABB corners to light space
        glm::vec3 corners[8] = {
            { mesh->aabb_min.x, mesh->aabb_min.y, mesh->aabb_min.z },
            { mesh->aabb_max.x, mesh->aabb_min.y, mesh->aabb_min.z },
            { mesh->aabb_min.x, mesh->aabb_max.y, mesh->aabb_min.z },
            { mesh->aabb_max.x, mesh->aabb_max.y, mesh->aabb_min.z },
            { mesh->aabb_min.x, mesh->aabb_min.y, mesh->aabb_max.z },
            { mesh->aabb_max.x, mesh->aabb_min.y, mesh->aabb_max.z },
            { mesh->aabb_min.x, mesh->aabb_max.y, mesh->aabb_max.z },
            { mesh->aabb_max.x, mesh->aabb_max.y, mesh->aabb_max.z },
        };

        // 
        for (auto &c : corners) {
            glm::vec4 world = cmd.transform * glm::vec4(c, 1.0f);
            glm::vec4 ls = light_view * world;
            ls_min = glm::min(ls_min, glm::vec3(ls));
            ls_max = glm::max(ls_max, glm::vec3(ls));
        }
    }
    float margin = 2.0f;
    float l = ls_min.x - margin;
    float r = ls_max.x + margin;
    float b = ls_min.y - margin;
    float t = ls_max.y + margin;
    float n = -ls_max.z - margin;
    float f = -ls_min.z + margin;
    
    glm::mat4 light_proj = glm::ortho(l, r, b, t, n, f);

    m_shadow_map.light_space_matrix = light_proj * light_view;

    // shadow pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_map.fbo_id);
    glViewport(0, 0, SYN_SHADOW_MAP_SIZE, SYN_SHADOW_MAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    shader->enable();
    shader->set_matrix_4fv("u_light_space_matrix", m_shadow_map.light_space_matrix);

    GLint current_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);
    
    for (uint32_t i = 0; i < m_command_count; i++) {
        const render_command_t &cmd = m_command_queue[i];
        mesh_internal_t *mesh = mesh_lib.get_mesh(cmd.mesh);
        if (!mesh) continue;
        shader->set_matrix_4fv("u_model", cmd.transform);
        mesh->vao.bind();
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, nullptr);
        mesh->vao.unbind();
    }

    // restore state
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    window_t *vp = window_manager.get_viewport_window();
    if (vp) {
        glm::vec2 sz = vp->get_content_size();
        glViewport(0, 0, sz.x, sz.y);
    }

}

// 
void renderer_t::set_shadow_ortho(float _size, float _z_near, float _z_far)
{
    m_shadow_map.ortho_size = _size;
    m_shadow_map.z_near     = _z_near;
    m_shadow_map.z_far      = _z_far;
    
}

//
void renderer_t::bind_scene_fbuffer(bool _update_viewport)
{
    window_t *vp = window_manager.get_viewport_window();
    if (!vp || !vp->has_frambuffer()) {
        SYN_WARNING("no viewport window framebuffer.\n");
        return;
    }

    m_scene_fbuffer_handle = vp->get_framebuffer_handle();
    
    framebuffer_t *fbo = api.fbo_handler.get_framebuffer(m_scene_fbuffer_handle);
    if (!fbo) {
        SYN_WARNING("invalid framebuffer.\n");
        return;
    }

    fbo->bind(_update_viewport);
    if (!_update_viewport) {
        glm::vec2 sz = vp->get_content_size();
        glViewport(0, 0, (GLsizei)sz.x, (GLsizei)sz.y);
    }
    // api clear color has to be set before this
    api.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    render_shadow_pass();
    bind_scene_fbuffer();

    if (m_do_render_skybox) render_skybox();
    
    //
    if (debug.show_wireframe) {
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

            shader->set_uniform_1i("u_shadows_enabled", m_shadow_map.is_active ? 1 : 0);
            if (m_shadow_map.is_active) {
                shader->set_matrix_4fv("u_light_space_matrix", m_shadow_map.light_space_matrix);
                shader->set_uniform_1i("u_shadow_map", 8);
            }
            
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

        // bind shadow map unconditionally
        glBindTextureUnit(8, m_shadow_map.depth_id);
    
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
    
    if (debug.show_wireframe) {
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
void renderer_t::render_ui_transform(const glm::vec3 &_world_pos)
{
    ui_transform_axis_t hovered_axis = editor.m_hovered_ui_transform_axis;
    
    float dist = glm::length(cam.get_position() - _world_pos);
    float axis_length = dist * 0.15f;

    entity_t *e = entity_lib.get_entity(selected_entity_handle);
    if (!e) return;

    ui_transform_mode_t mode = editor.get_ui_transform_mode();
    glm::vec3 axes[3];
    if (mode == ui_transform_mode_t::ROTATE) {
        axes[0] = glm::normalize(glm::vec3(e->transform[0]));
        axes[1] = glm::normalize(glm::vec3(e->transform[1]));
        axes[2] = glm::normalize(glm::vec3(e->transform[2]));
    } else {
        axes[0] = { 1.0f, 0.0f, 0.0f };
        axes[1] = { 0.0f, 1.0f, 0.0f };
        axes[2] = { 0.0f, 0.0f, 1.0f };
    }

    glm::vec4 colors[3] = {
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f, 1.0f },
    };

    // label
    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return;
    glm::vec2 vp_pos = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();
    glm::mat4 view = cam.get_view_matrix();
    glm::mat4 proj = cam.get_projection_matrix();
    glm::vec2 origin_ss = world_to_screen_fbo(e->t_position, 
                                              glm::vec2(0.0f), 
                                              vp_size, 
                                              view, 
                                              proj);

    float line_half_width = 2.5f;

    std::vector<ui_transform_vertex_t> vertices;
    std::vector<uint32_t> indices;

    auto to_ndc = [&](const glm::vec2 &p) -> glm::vec2 {
        return glm::vec2(
            p.x / vp_size.x * 2.0f - 1.0f,
            p.y / vp_size.y * 2.0f - 1.0f
        );
    };
    
    auto add_line_quad = [&](const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec4 &color) {
        glm::vec2 dir = p1 - p0;
        float len = glm::length(dir);
        if (len < 0.0001f) return;
        glm::vec2 n = glm::vec2(-dir.y, dir.x) / len * line_half_width;

        glm::vec2 a0 = to_ndc(p0 - n), a1 = to_ndc(p0 + n);
        glm::vec2 b0 = to_ndc(p1 - n), b1 = to_ndc(p1 + n);
        
        uint32_t base = (uint32_t)vertices.size();

        vertices.push_back({ a0, { 0.0f, -1.0f }, color });
        vertices.push_back({ b0, { 1.0f, -1.0f }, color });
        vertices.push_back({ b1, { 1.0f,  1.0f }, color });
        vertices.push_back({ a1, { 0.0f,  1.0f }, color });

        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
    };

    for (int i = 0; i < 3; i++) {
        glm::vec3 tip_world = _world_pos + axes[i] * axis_length;
        glm::vec2 tip_ss = world_to_screen_fbo(tip_world, glm::vec2(0.0f), vp_size, view, proj);

        glm::vec4 color = colors[i];
        if ((int)hovered_axis == i + 1) {
            color = glm::vec4(1.0f);
        }

        add_line_quad(origin_ss, tip_ss, color);
    }

    shader_t *shader = shader_lib.get_shader(m_ui_transform_shader_handle);
    if (!shader) return;

    api.disable_depth_test();
    shader->enable();

    m_ui_transform_vao.bind();
    m_ui_transform_vao.update_vertices((void *)&vertices[0], vertices.size() * sizeof(ui_transform_vertex_t));
    m_ui_transform_vao.update_indices((void *)&indices[0], indices.size() * sizeof(uint32_t));
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    m_ui_transform_vao.unbind();

    shader->disable();
    api.enable_depth_test();

    // label
    const char *mode_label = nullptr;
    switch (mode) {
        case ui_transform_mode_t::TRANSLATE:    mode_label = "[G] translate"; break;
        case ui_transform_mode_t::ROTATE:       mode_label = "[R] rotate"; break;
        case ui_transform_mode_t::SCALE:        mode_label = "[S] scale"; break;
    }
    
    // 
    if (mode_label) {
        float prev_depth = font.get_current_depth();
        glm::vec4 prev_color = font.get_color();
        font.set_depth(window_manager.get_znear());
        font.set_color(glm::vec4(1.0f));
        font.render_text(origin_ss.x + 10.0f, root_window.get_fheight() - origin_ss.y, "%s", mode_label);
        font.set_depth(prev_depth);
        font.set_color(prev_color);
    }
}

// 
void renderer_t::calculate_ui_projection_matrix()
{
    m_ui_projection = glm::ortho(0.0f, root_window.get_fwidth(),
                                root_window.get_fheight(), 0.0f,
                                window_manager.get_zfar(), window_manager.get_znear());
}

//
void renderer_t::init_debug_rendering() 
{
    debug.normal_shader_handle = shader_lib.load_from_file("debug_normal_shader", 
        "../assets/shaders/debug/debug_mesh_normals.glsl");
    
    debug.line_shader_handle = shader_lib.load_from_file("debug_line_shader", 
        "../assets/shaders/debug/debug_lines.glsl");
    
    // shader contains posistion (vec3) and color (vec4)
    size_t max_lines = 10000;
    size_t buffer_size = max_lines * 2 * (sizeof(glm::vec3) + sizeof(glm::vec4));

    debug.line_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 }
    });
    debug.line_vao.create_empty_vertices(buffer_size);
    
    // grid
    debug.grid_shader_handle = shader_lib.load_from_file("debug_grid_shader", 
        "../assets/shaders/debug/debug_grid.glsl");
    
    // create a custom vao, since no vbo is needed
    glGenVertexArrays(1, &debug.grid_vao_id);
    
    //
    m_debug_initialized = true;

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

void renderer_t::init_ui_rendering()
{
    // setup UI transform
    m_ui_transform_shader_handle = shader_lib.load_from_file("ui_transform_shader", 
        "../assets/shaders/ui_transform.glsl");

    size_t max_lines = 64;
    size_t vertices_size = max_lines * 4 * sizeof(ui_transform_vertex_t);
    size_t indices_size = max_lines * 6 * sizeof(uint32_t);
    

    m_ui_transform_vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_UV, shader_data_type_t::FLOAT2 },
        { VERTEX_ATTRIB_LOCATION_COLOR, shader_data_type_t::FLOAT4 }
    });
    m_ui_transform_vao.create_empty_vertices(vertices_size);
    m_ui_transform_vao.create_empty_indices(indices_size);

    // ui projection and misc shaders and vaos
    calculate_ui_projection_matrix();

    m_ui_tex_quad_shader_handle = shader_lib.load_from_file("ui_tex_quad_shader", 
        "../assets/shaders/ui_quad_tex.glsl");

    // 
    glm::vec2 vs[] = { 
        { 0.0f, 0.0f },
        { 0.0f, 1.0f }, 
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
    };
    uint32_t is[] = { 0, 1, 2, 2, 3, 0 };
    m_ui_tex_quad_vao.set_buffer_layout({ { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT2 } });
    m_ui_tex_quad_vao.create(vs, 4, is, 6);

    // 
    m_ui_color_picker_shader_handle = shader_lib.load_from_file("ui_color_picker_shader", 
        "../assets/shaders/ui_color_picker.glsl");
    
    
}

// 
void renderer_t::shutdown_ui_rendering()
{
   	m_ui_transform_vao.destroy();
    m_ui_tex_quad_vao.destroy();

}

//
void renderer_t::toggle_wireframe()      { debug.show_wireframe      = !debug.show_wireframe;       }
void renderer_t::toggle_normals()        { debug.show_normals        = !debug.show_normals;         }
void renderer_t::toggle_tangents()       { debug.show_tangents       = !debug.show_tangents;        }
void renderer_t::toggle_bounding_boxes() { debug.show_bounding_boxes = !debug.show_bounding_boxes;  }
void renderer_t::toggle_grid()           { debug.show_grid           = !debug.show_grid;            }

//
void renderer_t::render_debug_normals(mesh_handle_t _mesh_handle, const glm::mat4 &_transform)
{
    if (!debug.show_normals && !debug.show_tangents) return;
    if (!m_debug_initialized) return;
    
    mesh_internal_t *mesh = mesh_lib.get_mesh(_mesh_handle);
    if (!mesh) return;
    
    shader_t *shader = shader_lib.get_shader(debug.normal_shader_handle);
    if (!shader) return;
    
    shader->enable();
    
    glm::mat4 vp = cam.get_view_projection_matrix();
    glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(_transform)));
    
    shader->set_matrix_4fv("u_model", _transform);
    shader->set_matrix_4fv("u_view_projection", vp);
    shader->set_matrix_3fv("u_normal_matrix", normal_matrix);
    shader->set_uniform_1f("u_normal_length", debug.normal_length);
    shader->set_uniform_1i("u_show_tangents", debug.show_tangents ? 1 : 0);
    
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
    if (!debug.show_bounding_boxes) return;
    if (!m_debug_initialized) init_debug_rendering();
    
    // glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 color(1.0f, 0.7f, 0.05f, 1.0f);
    
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
    
    debug.line_vao.bind();
    debug.line_vao.update_vertices((void *)&lines[0], lines.size() * sizeof(float));
    
    shader_t *shader = shader_lib.get_shader(debug.line_shader_handle);
    if (!shader) return;
    
    shader->enable();
    shader->set_matrix_4fv("u_view_projection", cam.get_view_projection_matrix());

    // glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, lines.size() / 7);
    // glLineWidth(1.0f);
    
    debug.line_vao.unbind();
    
    m_perf_stats.draw_calls_per_frame++;
    
}

//
void renderer_t::render_debug_grid(float _y_level)
{
    if (!debug.show_grid) return;
    
    shader_t *shader = shader_lib.get_shader(debug.grid_shader_handle);
    if (!shader) return;
    
    glBindVertexArray(debug.grid_vao_id);
    
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
void renderer_t::render_orientation_obj()
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


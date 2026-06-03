#ifndef __RENDERER_TYPES_H
#define __RENDERER_TYPES_H

#include <stdint.h>
#include <glm/glm.hpp>

#include "renderer/mesh/mesh_types.h"
#include "renderer/material/material_types.h"
#include "renderer/shader/shader.h"
#include "renderer/buffers/vertex_array.h"

// 
#define SYN_PERF_GRAPH_SAMPLE_COUNT  100

// 
struct uniform_buffer_t {
    uint32_t opengl_id = 0;
    uint32_t binding_point = 0;
    size_t size = 0;
};

//
struct render_command_t {
    mesh_handle_t mesh;
    material_handle_t material;
    glm::mat4 transform;
};

// 
struct perf_stats_t {
    bool show_overlay = true;
    bool show_graph = true;
    vertex_array_t graph_vao;
    bool graph_vao_initialized = false;
    shader_t *graph_shader = NULL;
    uint32_t draw_calls_per_frame = 0;

    float frame_times[SYN_PERF_GRAPH_SAMPLE_COUNT];
    uint32_t frame_time_idx = 0;
    float max_frame_time = 16.67f;
    
};

// 
struct debug_state_t {
    bool show_wireframe      = false;
    bool show_normals        = false;
    bool show_tangents       = false;
    bool show_bounding_boxes = false;
    bool show_grid           = false;

    float normal_length      = 0.1f;
    float tangent_length     = 0.1f;

    vertex_array_t debug_line_vao;
    shader_t *debug_line_shader;
    shader_t *debug_normal_shader;    
};


#endif // __RENDERER_TYPES_H

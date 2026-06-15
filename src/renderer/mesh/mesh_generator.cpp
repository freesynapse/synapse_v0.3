
#include <vector>

#include "renderer/mesh/mesh_generator.h"

#include "c_api.h"

// 
mesh_handle_t generate_cube_mesh()
{
    // 1. Define the 24 vertices for a clean 3D Cube (6 faces * 4 vertices)
    // This gives each face its own unique normal for correct lighting reflections.
    vertex_data_t cube_vertices[] = {
        // Front Face (Normal +Z)
        // Pos                      Normal              Tangent             Bitangent           UV
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },

        // Back Face (Normal -Z)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },

        // Top Face (Normal +Y)
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },

        // Bottom Face (Normal -Y)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },

        // Right Face (Normal +X)
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },

        // Left Face (Normal -X)
        { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.1f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} }
    };

    // 2. Define the 36 indices forming the 12 triangles of the cube
    uint32_t cube_indices[] = {
        0, 1, 2,    0, 2, 3,    // Front
        4, 5, 6,    4, 6, 7,    // Back
        8, 9, 10,   8, 10, 11,  // Top
        12, 13, 14, 12, 14, 15, // Bottom
        16, 17, 18, 16, 18, 19, // Right
        20, 21, 22, 20, 22, 23  // Left
    };

    uint32_t vertex_count = sizeof(cube_vertices) / sizeof(vertex_data_t);
    uint32_t index_count = sizeof(cube_indices) / sizeof(uint32_t);

    // 3. Upload to the handle-based library
    return mesh_lib.load_mesh("primitive_cube", cube_vertices, vertex_count, cube_indices, index_count);
    
}

// only position
mesh_handle_t generate_skybox_cube_mesh()
{
    glm::vec3 cube_vertices[] = {
        {-0.5f, -0.5f,  0.5f}, 
        { 0.5f, -0.5f,  0.5f}, 
        { 0.5f,  0.5f,  0.5f}, 
        {-0.5f,  0.5f,  0.5f}, 
        {-0.5f, -0.5f, -0.5f}, 
        {-0.5f,  0.5f, -0.5f}, 
        { 0.5f,  0.5f, -0.5f}, 
        { 0.5f, -0.5f, -0.5f}, 
        {-0.5f,  0.5f, -0.5f}, 
        {-0.5f,  0.5f,  0.5f}, 
        { 0.5f,  0.5f,  0.5f}, 
        { 0.5f,  0.5f, -0.5f}, 
        {-0.5f, -0.5f, -0.5f}, 
        { 0.5f, -0.5f, -0.5f}, 
        { 0.5f, -0.5f,  0.5f}, 
        {-0.5f, -0.5f,  0.5f}, 
        { 0.5f, -0.5f, -0.5f}, 
        { 0.5f,  0.5f, -0.5f}, 
        { 0.5f,  0.5f,  0.5f}, 
        { 0.5f, -0.5f,  0.5f}, 
        {-0.5f, -0.5f, -0.5f}, 
        {-0.5f, -0.5f,  0.5f}, 
        {-0.5f,  0.5f,  0.5f}, 
        {-0.5f,  0.5f, -0.5f}, 
    };

    uint32_t cube_indices[] = {
        2, 1, 0,    3, 2, 0,    // Front
        6, 5, 4,    7, 6, 4,    // Back
        10, 9, 8,   11, 10, 8,  // Top
        14, 13, 12, 15, 14, 12, // Bottom
        18, 17, 16, 19, 18, 16, // Right
        22, 21, 20, 23, 22, 20  // Left
    };
    vertex_array_t vao;
    vao.set_buffer_layout({ 
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 },
    });
    vao.create(cube_vertices, 
               sizeof(cube_vertices) / sizeof(glm::vec3), 
               cube_indices, 
               sizeof(cube_indices) / sizeof(uint32_t)
    );
    
    return mesh_lib.load_mesh_from_vao("skybox", vao);
    
}

// 
mesh_handle_t generate_uv_sphere_mesh(float _radius, uint32_t _sectors, uint32_t _stacks)
{
    std::vector<vertex_data_t> vertices;
    std::vector<uint32_t> indices;

    float sector_step = 2.0f * M_PI / _sectors;
    float stack_step = M_PI / _stacks;

    // generate vertex data
    for (uint32_t i = 0; i <= _stacks; i++) {
        float stack_angle = M_PI * 0.5f - (i * stack_step);
        float xy = _radius * cosf(stack_angle);
        float z = _radius * sinf(stack_angle);

        for (uint32_t j = 0; j <= _sectors; j++) {
            float sector_angle = j * sector_step;

            vertex_data_t vertex;

            //
            vertex.position.x = xy * cosf(sector_angle);
            vertex.position.y = xy * sinf(sector_angle);
            vertex.position.z = z;

            // 
            vertex.normal = glm::normalize(vertex.position);

            // 
            vertex.uv.x = (float)j / _sectors;
            vertex.uv.y = (float)i / _sectors;

            // tangent -- derivative aling the longitudinal direction
            vertex.tangent.x = -sinf(sector_angle);
            vertex.tangent.y = cosf(sector_angle);
            vertex.tangent.z = 0.0f;
            vertex.tangent = glm::normalize(vertex.tangent);

            // bitangent -- derivative aling the latitudinal direction
            // also, the cross between the normal and the tangent..
            vertex.bitangent = glm::normalize(glm::cross(vertex.normal, vertex.tangent));

            vertices.push_back(vertex);
        }
    }

    // generate indices
    for (uint32_t i = 0; i < _stacks; i++) {
        uint32_t k1 = i * (_sectors + 1);
        uint32_t k2 = k1 + _sectors + 1;

        for (uint32_t j = 0; j < _sectors; j++, k1++, k2++) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (_stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    vertex_array_t vao;

    vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION,  shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_NORMAL,    shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_TANGENT,   shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_BITANGENT, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_UV,        shader_data_type_t::FLOAT2 },
    });
    vao.create(&vertices[0], vertices.size(), &indices[0], indices.size());

    return mesh_lib.load_mesh_from_vao("sphere_uv", vao, (void *)&vertices[0]);
    
}

// 
mesh_handle_t generate_plane_mesh(float _size, uint32_t _subdivisions)
{
    std::vector<vertex_data_t> vertices;
    std::vector<uint32_t> indices;

    uint32_t n = _subdivisions + 1;
    float step = _size / _subdivisions;
    float uv_step = 1.0f / _subdivisions;
    float half = _size * 0.5f;

    for (uint32_t row = 0; row <= _subdivisions; row++) {
        for (uint32_t col = 0; col <= _subdivisions; col++) {
            vertex_data_t v;
            v.position  = { -half + col * step, 0.0f, -half + row * step };
            v.normal    = { 0.0f, 1.0f, 0.0f };
            v.tangent   = { 1.0f, 0.0f, 0.0f };
            v.bitangent = { 0.0f, 0.0f, 1.0f };
            v.uv        = { col * uv_step, row * uv_step };
            vertices.push_back(v);
        }
    }

    for (uint32_t row = 0; row < _subdivisions; row++) {
        for (uint32_t col = 0; col < _subdivisions; col++) {
            uint32_t i = row * n + col;
            indices.push_back(i);
            indices.push_back(i + n);
            indices.push_back(i + 1);
            indices.push_back(i + 1);
            indices.push_back(i + n);
            indices.push_back(i + n + 1);
        }
    }

    vertex_array_t vao;
    vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION,  shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_NORMAL,    shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_TANGENT,   shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_BITANGENT, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_UV,        shader_data_type_t::FLOAT2 },
    });
    vao.create(&vertices[0], vertices.size(), &indices[0], indices.size());

    return mesh_lib.load_mesh_from_vao("primitive_plane", vao, (void *)&vertices[0]);
    
}

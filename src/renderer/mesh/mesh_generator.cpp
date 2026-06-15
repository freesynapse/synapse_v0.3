
#include <vector>

#include "renderer/mesh/mesh_generator.h"
#include "utils/log.h"

#include "c_api.h"

// 
mesh_handle_t mesh_generator_t::create_cube_mesh()
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
mesh_handle_t mesh_generator_t::create_skybox_cube_mesh()
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
mesh_handle_t mesh_generator_t::create_uv_sphere_mesh(float _radius, uint32_t _sectors, uint32_t _stacks)
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
mesh_handle_t mesh_generator_t::create_plane_mesh(float _size, uint32_t _subdivisions)
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

// 
mesh_handle_t mesh_generator_t::create_cylinder_mesh(float _radius, float _height, uint32_t _sectors)
{
    return _create_cylinder_or_cone("cylinder", _radius, _radius, _height, _sectors);
    
}

// 
mesh_handle_t mesh_generator_t::create_cone_mesh(float _radius, float _height, uint32_t _sectors)
{
    return _create_cylinder_or_cone("cone", _radius, 0.0f, _height, _sectors);
    
}

// 
mesh_handle_t mesh_generator_t::create_torus_mesh(float _outer_radius,
                                                  float _inner_radius,
                                                  uint32_t _sectors,
                                                  uint32_t _sides)
{
    std::vector<vertex_data_t> vertices;
    std::vector<uint32_t> indices;

    float sector_step = 2.0f * M_PI / _sectors;
    float side_step   = 2.0f * M_PI / _sides;

    for (uint32_t i = 0; i <= _sectors; i++) {
        float phi = i * sector_step;
        glm::vec3 ring_center = { cosf(phi) * _outer_radius, 0.0f, sinf(phi) * _outer_radius };
        // ring tangent (around the big circle)
        glm::vec3 ring_dir = glm::normalize(glm::vec3(-sinf(phi), 0.0f, cosf(phi)));

        for (uint32_t j = 0; j <= _sides; j++) {
            float theta = j * side_step;

            // outward direction in the tube cross-section plane
            glm::vec3 radial = glm::normalize(glm::vec3(cosf(phi), 0.0f, sinf(phi)));
            glm::vec3 up     = { 0.0f, 1.0f, 0.0f };
            glm::vec3 tube_dir = cosf(theta) * radial + sinf(theta) * up;

            vertex_data_t v;
            v.position  = ring_center + tube_dir * _inner_radius;
            v.normal    = tube_dir;
            v.tangent   = ring_dir;
            v.bitangent = glm::normalize(glm::cross(v.normal, v.tangent));
            v.uv        = { (float)i / _sectors, (float)j / _sides };
            vertices.push_back(v);
        }
    }

    uint32_t stride = _sides + 1;
    for (uint32_t i = 0; i < _sectors; i++) {
        for (uint32_t j = 0; j < _sides; j++) {
            uint32_t a = i * stride + j;
            uint32_t b = a + stride;
            indices.push_back(a);     indices.push_back(a + 1); indices.push_back(b);
            indices.push_back(a + 1); indices.push_back(b + 1); indices.push_back(b);
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
    SYN_INFO("created torys\n");
    return mesh_lib.load_mesh_from_vao("primitive_torus", vao, (void *)&vertices[0]);

}

// shared internal helper
mesh_handle_t mesh_generator_t::_create_cylinder_or_cone(const std::string &_name,
                                                         float _bottom_radius,
                                                         float _top_radius,
                                                         float _height,
                                                         uint32_t _sectors)
{
    std::vector<vertex_data_t> vertices;
    std::vector<uint32_t> indices;

    float sector_step = 2.0f * M_PI / _sectors;
    float half_h = _height * 0.5f;
    float slope = (_bottom_radius - _top_radius) / _height;   // for outward normal tilt

    // side vertices — two rings (bottom + top), duplicated per sector for clean normals
    for (uint32_t i = 0; i <= _sectors; i++) {
        float angle = i * sector_step;
        float cx = cosf(angle);
        float cy = sinf(angle);

        // outward normal accounts for the cone taper
        glm::vec3 outward = glm::normalize(glm::vec3(cx, 0.0f, cy));
        glm::vec3 normal  = glm::normalize(glm::vec3(outward.x, slope, outward.z));
        glm::vec3 tangent = glm::normalize(glm::vec3(-sinf(angle), 0.0f, cosf(angle)));
        glm::vec3 bitan   = glm::normalize(glm::cross(normal, tangent));
        float u = (float)i / _sectors;

        // bottom ring
        vertex_data_t vb;
        vb.position  = { _bottom_radius * cx, -half_h, _bottom_radius * cy };
        vb.normal    = normal;
        vb.tangent   = tangent;
        vb.bitangent = bitan;
        vb.uv        = { u, 0.0f };
        vertices.push_back(vb);

        // top ring
        vertex_data_t vt;
        vt.position  = { _top_radius * cx, half_h, _top_radius * cy };
        vt.normal    = normal;
        vt.tangent   = tangent;
        vt.bitangent = bitan;
        vt.uv        = { u, 1.0f };
        vertices.push_back(vt);
    }

    // side indices
    for (uint32_t i = 0; i < _sectors; i++) {
        uint32_t b0 = i * 2,     t0 = b0 + 1;
        uint32_t b1 = b0 + 2,    t1 = b0 + 3;
        indices.push_back(b0); indices.push_back(t0); indices.push_back(b1);
        indices.push_back(t0); indices.push_back(t1); indices.push_back(b1);
    }

    // cap helper lambda
    auto add_cap = [&](float _radius, float _y, bool _flip) {
        uint32_t center_idx = (uint32_t)vertices.size();
        glm::vec3 cap_normal = _flip ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
        glm::vec3 cap_tan    = { 1.0f, 0.0f, 0.0f };
        glm::vec3 cap_bitan  = { 0.0f, 0.0f, _flip ? -1.0f : 1.0f };

        vertex_data_t center;
        center.position  = { 0.0f, _y, 0.0f };
        center.normal    = cap_normal;
        center.tangent   = cap_tan;
        center.bitangent = cap_bitan;
        center.uv        = { 0.5f, 0.5f };
        vertices.push_back(center);

        for (uint32_t i = 0; i <= _sectors; i++) {
            float angle = i * sector_step;
            float cx = cosf(angle), cy = sinf(angle);
            vertex_data_t v;
            v.position  = { _radius * cx, _y, _radius * cy };
            v.normal    = cap_normal;
            v.tangent   = cap_tan;
            v.bitangent = cap_bitan;
            v.uv        = { cx * 0.5f + 0.5f, cy * 0.5f + 0.5f };
            vertices.push_back(v);
        }

        for (uint32_t i = 0; i < _sectors; i++) {
            uint32_t a = center_idx + 1 + i;
            uint32_t b = a + 1;
            if (_flip) { indices.push_back(center_idx); indices.push_back(a); indices.push_back(b); }
            else       { indices.push_back(center_idx); indices.push_back(b); indices.push_back(a); }
        }
    };

    add_cap(_bottom_radius, -half_h, true);    // bottom cap
    if (_top_radius > 0.001f)
        add_cap(_top_radius, half_h, false);   // top cap (skipped for cone tip)

    vertex_array_t vao;
    vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION,  shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_NORMAL,    shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_TANGENT,   shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_BITANGENT, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_UV,        shader_data_type_t::FLOAT2 },
    });
    vao.create(&vertices[0], vertices.size(), &indices[0], indices.size());
    return mesh_lib.load_mesh_from_vao(_name, vao, (void *)&vertices[0]);
}


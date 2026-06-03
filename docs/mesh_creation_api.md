Code flow for a simple mesh/shader/material setup:

1. Create a VAO from 

    struct data_ {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    // vertex data and indices
    data_ vertices[] = { ... };
    uint32_t indices[] = { ... };

    // Creating a simple quad or mesh with just position and UV
    vertex_array_t vao;
    vao.set_buffer_layout({
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 }, // location 0
        { VERTEX_ATTRIB_LOCATION_UV,       shader_data_type_t::FLOAT2 }  // location 4
    });
  
    vao.create(vertices, 
               sizeof(vertices) / sizeof(data_), 
               indices, 
               sizeof(indices) / sizeof(uint32_t)
    );

    // Load it into the library
    mesh_handle_t simple_mesh = mesh_lib.load_mesh_from_vao("my_simple_mesh", vao);


2. Shader and material setup

    // setup the shader
    shader_handle = shader_lib.load_from_file("simple_shader", "../assets/shaders/simpel.glsl");
    shader_t *shader = shader_lib.get(shader_handle);

    // create the material using the simple shader
    material_handle_t mat_handle = mat_lib.create_material(shader->get_id());
    material_internal_t *mat = mat_lib.get_material(mat_h);
    
    // set data_size to 0 since this shader doesn't use the material UBO
    mat->data_size = 0;
    
    // bind your texture to the first slot (slot 0)
    mat->textures[0] = some_texture_handle;


3. Rendering

In the rendering loop of the implementation:

    renderer.submit_mesh(mesh_handle, material_handle, model_matrix);
    renderer.flush_commands();



**Appendix A -- generation of a cube mesh with custom vertex data**
// Testing how the buffer layout could interface with the mesh library.
mesh_handle_t example_gen_cube()
{
    struct __test {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    __test cube_vertices[] = {
        // Front Face (Normal pointing +Z)
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },

        // Back Face (Normal pointing -Z)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },

        // Top Face (Normal pointing +Y)
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },

        // Bottom Face (Normal pointing -Y)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },

        // Right Face (Normal pointing +X)
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },

        // Left Face (Normal pointing -X)
        { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }
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

    vertex_array_t vao;
    vao.set_buffer_layout({ 
        { VERTEX_ATTRIB_LOCATION_POSITION, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_NORMAL, shader_data_type_t::FLOAT3 },
        { VERTEX_ATTRIB_LOCATION_UV, shader_data_type_t::FLOAT2 },
    });
    vao.create(cube_vertices, 
               sizeof(cube_vertices) / sizeof(__test), 
               cube_indices, 
               sizeof(cube_indices) / sizeof(uint32_t)
    );
    
    return mesh_lib.load_mesh_from_vao("test_buffer_layout", vao);
    
}


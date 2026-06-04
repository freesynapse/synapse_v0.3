#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 4) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform mat3 u_normal_matrix;

out vec3 v_normal;
out vec2 v_uv;

// 
void main()
{
    v_normal = u_normal_matrix * a_normal;
    v_uv = a_uv;
    gl_Position = u_view_projection * u_model * vec4(a_position, 1.0);
}


#type FRAGMENT_SHADER
#version 450 core

in vec3 v_normal;
in vec2 v_uv;

layout (std140, binding=1) uniform u_material_block {
    vec4  u_material_albedo_color;
    vec4  u_material_emissive_color;
    
    float u_material_roughness;
    float u_material_metallic;
    float u_material_ao;
    float u_material_tiling_factor;

    float u_use_albedo_map;
    float u_use_normal_map;
    float u_use_metallic_map;
    float u_use_roughness_map;
};

layout(binding=0) uniform sampler2D u_albedo_texture;

out vec4 frag_color;

// 
void main()
{
    vec3 N = normalize(v_normal);

    // temp static light dir
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));

    // ambient = 0.2
    float diffuse = max(dot(N, L), 0.3);

    vec4 tex_color = texture(u_albedo_texture, v_uv * u_material_tiling_factor);
    // vec4 tex_color = texture(u_albedo_texture, v_uv);
    vec3 final_color = tex_color.rgb * u_material_albedo_color.rgb * diffuse;
    // vec3 final_color = tex_color.rgb * diffuse;
    // float force_retention = (u_material_roughness + u_material_metallic + u_material_ao) * 0.000001;
    // 
    // frag_color = vec4(final_color + force_retention, 1.0);
    frag_color = vec4(final_color, 1.0);
    
}


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

uniform vec4 u_material_albedo_color;
uniform float u_material_roughness;
uniform float u_material_metallic;
uniform float u_material_tiling_factor;

uniform sampler2D u_albedo_texture;

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
    vec3 final_color = tex_color.rgb * u_material_albedo_color.rgb * diffuse;

    frag_color = vec4(final_color, 1.0);
    
}




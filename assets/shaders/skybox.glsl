#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;

out vec3 v_uv;

uniform mat4 u_view;
uniform mat4 u_projection;

// 
void main()
{
    v_uv = a_position;

    // remove translation from view matrix
    mat4 view = mat4(mat3(u_view));
    vec4 pos = u_projection * view * vec4(a_position, 1.0);

    // force z to be 1.0 (furthest possible depth)
    gl_Position = pos.xyww;
    
}

#type FRAGMENT_SHADER
#version 450 core

out vec4 frag_color;

in vec3 v_uv;

uniform samplerCube u_skybox_sampler;

// 
void main()
{
    frag_color = texture(u_skybox_sampler, v_uv);
    
}
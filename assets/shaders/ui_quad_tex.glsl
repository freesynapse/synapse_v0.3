#type VERTEX_SHADER
#version 450 core

layout(location=0) in vec2 a_vertex;

uniform mat4 u_projection;
uniform vec2 u_position;
uniform vec2 u_size;
uniform float u_depth;

out vec2 v_uv;

// 
void main()
{
    vec2 pos = u_position + a_vertex * u_size;
    gl_Position = u_projection * vec4(pos, u_depth, 1.0);
    v_uv = vec2(a_vertex.x, 1.0 - a_vertex.y);
    
}


#type FRAGMENT_SHADER
#version 450 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_texture_sampler;

// 
void main()
{
    vec3 tex = texture(u_texture_sampler, v_uv).rgb;
    frag_color = vec4(tex, 1.0);
}

#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;

// 
void main()
{
    vec2 p = 2.0 * a_position.xy - 1.0;
    gl_Position = vec4(p.x, p.y, a_position.z, 1.0);
}


#type FRAGMENT_SHADER
#version 450 core

uniform vec4 u_color;
layout(location = 0) out vec4 frag_color;

// 
void main()
{
    frag_color = u_color;

}

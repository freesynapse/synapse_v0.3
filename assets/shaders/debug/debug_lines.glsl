#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 5) in vec4 a_color;

uniform mat4 u_view_projection;

out vec4 v_color;

// 
void main()
{
    gl_Position = u_view_projection * vec4(a_position, 1.0);
    v_color = a_color;
    
}


#type FRAGMENT_SHADER
#version 450 core

in vec4 v_color;
out vec4 frag_color;

// 
void main()
{
    frag_color = v_color;
    
}

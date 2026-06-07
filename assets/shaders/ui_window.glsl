#type VERTEX_SHADER
#version 450 core

layout(location=0) in vec2 a_pos;
layout(location=5) in vec4 a_color;
layout(location=6) in float a_depth;

out vec4 v_color;

uniform mat4 u_projection;

// 
void main() {
    gl_Position = u_projection * vec4(a_pos, a_depth, 1.0);
    v_color = a_color;
    
}


#type FRAGMENT_SHADER
#version 450 core

in vec4 v_color;

out vec4 frag_color;

// 
void main() {
    frag_color = v_color;
    
}

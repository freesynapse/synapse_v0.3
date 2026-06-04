#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_pos;
layout(location = 5) in vec4 a_color;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec4 v_color;

// 
void main() {
    gl_Position = u_projection * u_model * vec4(a_pos, 0.0, 1.0);
    v_color = a_color;
}


#type FRAGMENT_SHADER
#version 450 core

in vec4 v_color;
out vec4 frag_color;
uniform vec4 u_color;

// 
void main() {
    // Use vertex color if present, otherwise uniform
    frag_color = v_color.a > 0.0 ? v_color : u_color;

}
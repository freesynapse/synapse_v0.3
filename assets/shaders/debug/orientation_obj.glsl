#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 5) in vec3 a_color;

layout(location = 0) out vec3 v_color;

uniform mat4 u_mvp;

// 
void main() {
    v_color = a_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}

#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) in vec3 v_color;
layout(location = 0) out vec4 frag_color;

// 
void main() {
    frag_color = vec4(v_color, 1.0);
    // frag_color = vec4(1.0);

}
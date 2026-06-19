#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;    // quad vertex in screen space
layout(location = 4) in vec2 a_uv;          // -1..1 across line width
layout(location = 5) in vec4 a_color;

out vec2 v_uv;
out vec4 v_color;

// uniform mat4 u_projection;

// 
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    // gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
    v_uv = a_uv;
    v_color = a_color;

}


#type FRAGMENT_SHADER
#version 450 core

in vec2 v_uv;
in vec4 v_color;
out vec4 frag_color;

// 
void main()
{
    // v_uv.y is signed distance from line center in [-1, 1]
    float dist = abs(v_uv.y);
    float alpha = 1.0 - smoothstep(0.6, 1.0, dist);
    if (alpha < 0.01) discard;
    frag_color = vec4(v_color.rgb, v_color.a * alpha);

}

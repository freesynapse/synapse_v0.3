#type VERTEX_SHADER
#version 450 core

layout(location=0) out vec2 v_uv;

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);

    v_uv.x = (x + 1.0) * 0.5;
    v_uv.y = (y + 1.0) * 0.5;

    gl_Position = vec4(x, y, 0.0, 1.0);
    
}

#type FRAGMENT_SHADER
#version 450 core

layout(location=0) in vec2 v_uv;
layout(location=0) out vec4 frag_color;

uniform sampler2D u_texture_sampler;

void main()
{
    vec3 tex = texture(u_texture_sampler, v_uv).rgb;
	frag_color = vec4(tex, 1.0);
}

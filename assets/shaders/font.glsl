#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 4) in vec2 a_uv;
layout(location = 5) in vec4 a_color;

out vec2 v_uv;
out vec4 v_color;

void main()
{
	gl_Position = vec4(a_position, 0.0f, 1.0f);
	v_uv = a_uv;
	v_color = a_color;
}


#type FRAGMENT_SHADER
#version 450 core

in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

layout(binding=0) uniform sampler2D u_font_atlas;

void main()
{
	float a = texture(u_font_atlas, v_uv).r;
	frag_color = vec4(vec3(v_color.rgb), a * v_color.a);
	
}

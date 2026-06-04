#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;

out vec2 f_tex_pos;

void main()
{
	gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
	f_tex_pos = a_position.zw;
}


#type FRAGMENT_SHADER
#version 330 core

in vec2 f_tex_pos;

out vec4 frag_color;

uniform sampler2D u_texture_sampler;
uniform vec4 u_color;

void main()
{
	float a = texture2D(u_texture_sampler, f_tex_pos).r;
	frag_color = vec4(u_color.rgb, a * u_color.a);
	
}

#type VERTEX_SHADER
#version 450 core

layout(location=0) in vec2 a_vertex;

uniform mat4  u_projection;
uniform vec2  u_position;
uniform vec2  u_size;
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

uniform int   u_mode;   // 0 = SV square, 1 = hue strip
uniform float u_hue;    // 0..1, only used in SV mode

// 
vec3 hsv_to_rgb(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0 - abs(mod(h * 6.0, 2.0) - 1.0));
    float m = v - c;
    vec3 rgb;
    if      (h < 1.0/6.0) rgb = vec3(c, x, 0.0);
    else if (h < 2.0/6.0) rgb = vec3(x, c, 0.0);
    else if (h < 3.0/6.0) rgb = vec3(0.0, c, x);
    else if (h < 4.0/6.0) rgb = vec3(0.0, x, c);
    else if (h < 5.0/6.0) rgb = vec3(x, 0.0, c);
    else                   rgb = vec3(c, 0.0, x);
    return rgb + m;
}

// 
void main()
{
    if (u_mode == 0) {
        // SV square: x=saturation, y=value
        frag_color = vec4(hsv_to_rgb(u_hue, v_uv.x, v_uv.y), 1.0);
    } else {
        // hue strip: y maps to hue
        frag_color = vec4(hsv_to_rgb(v_uv.y, 1.0, 1.0), 1.0);
    }

}


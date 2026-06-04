#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;

out vec3 v_uv;

uniform mat4 u_view;
uniform mat4 u_projection;

// 
void main()
{
    v_uv = a_position;

    // remove translation from view matrix
    mat4 view = mat4(mat3(u_view));
    vec4 pos = u_projection * view * vec4(a_position, 1.0);

    // force z to be 1.0 (furthest possible depth)
    gl_Position = pos.xyww;
    
}

#type FRAGMENT_SHADER
#version 450 core
out vec4 frag_color;
in vec3 v_uv;

uniform sampler2D u_equirectangular_map;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // 1/(2*PI), 1/PI
    uv += 0.5;
    return uv;
}

// 
void main()
{
    vec2 uv = SampleEquirectangular(normalize(v_uv));
    vec3 color = texture(u_equirectangular_map, uv).rgb;
    frag_color = vec4(color, 1.0);
    // frag_color = vec4(1.0);
}

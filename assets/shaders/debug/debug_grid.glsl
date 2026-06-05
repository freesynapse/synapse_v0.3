#type VERTEX_SHADER
#version 450 core

out vec3 v_near_point;
out vec3 v_far_point;

// Full-screen quad in NDC
const vec2 positions[4] = vec2[](
    vec2(-1, -1), vec2( 1, -1),
    vec2(-1,  1), vec2( 1,  1)
);

uniform mat4 u_inv_view_projection;

// 
vec3 unproject(vec2 ndc, float z)
{
    vec4 p = u_inv_view_projection * vec4(ndc, z, 1.0);
    return p.xyz / p.w;
}

// 
void main()
{
    vec2 ndc = positions[gl_VertexID];
    v_near_point = unproject(ndc, 0.0);
    v_far_point  = unproject(ndc, 1.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
}


#type FRAGMENT_SHADER
#version 450 core

in vec3 v_near_point;
in vec3 v_far_point;

out vec4 frag_color;

uniform mat4 u_view_projection;
uniform float u_near;
uniform float u_far;

// 
vec4 grid(vec3 pos, float scale, bool draw_axes)
{
    vec2 coord = pos.xz / scale;
    vec2 deriv = fwidth(coord);          // rate of change in screen space
    vec2 grid  = abs(fract(coord - 0.5) - 0.5) / deriv;
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);

    vec4 color = vec4(0.4, 0.4, 0.4, alpha);

    if (draw_axes)
    {
        // X axis (red) — highlight where Z ≈ 0
        float x_axis = abs(pos.z) / (deriv.y * scale);
        if (x_axis < 1.0)
            color = vec4(1.0, 0.2, 0.2, 1.0);

        // Z axis (blue) — highlight where X ≈ 0
        float z_axis = abs(pos.x) / (deriv.x * scale);
        if (z_axis < 1.0)
            color = vec4(0.2, 0.4, 1.0, 1.0);
    }

    return color;
}

// 
float linearize_depth(float d)
{
    return (2.0 * u_near * u_far) / (u_far + u_near - (2.0 * d - 1.0) * (u_far - u_near));
}

// 
float fade(vec3 pos)
{
    float d = length(pos.xz);   // distance from camera in XZ plane
    return 1.0 - smoothstep(u_far * 0.3, u_far * 0.8, d);
}

// 
void main()
{
    // Ray-plane intersection at y = 0
    float t = -v_near_point.y / (v_far_point.y - v_near_point.y);
    vec3  pos = v_near_point + t * (v_far_point - v_near_point);

    if (t < 0.0) discard;   // intersection is behind camera

    // Two grid scales: coarse on top of fine
    vec4 c1 = grid(pos, 1.0,  true);   // 1-unit grid, draws axes
    vec4 c2 = grid(pos, 10.0, false);  // 10-unit grid, no axes

    float f = fade(pos);
    frag_color = (c1 + c2 * 0.4) * f;
    frag_color.a *= f;

    if (frag_color.a < 0.001) discard;

    // Write correct depth so the grid occludes geometry properly
    vec4 clip = u_view_projection * vec4(pos, 1.0);
    gl_FragDepth = (clip.z / clip.w) * 0.5 + 0.5;
}

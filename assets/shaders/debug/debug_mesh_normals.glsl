#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;

uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform mat3 u_normal_matrix;

out VS_OUT {
    vec3 world_pos;    // NEW: Pass world position
    vec3 normal;
    vec3 tangent;
} vs_out;

// 
void main()
{
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    vs_out.world_pos = world_pos.xyz;  // NEW
    
    vs_out.normal = normalize(u_normal_matrix * a_normal);
    vs_out.tangent = normalize(u_normal_matrix * a_tangent);
    
    gl_Position = u_view_projection * world_pos;
}


#type GEOMETRY_SHADER
#version 450 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 world_pos;    // NEW
    vec3 normal;
    vec3 tangent;
} gs_in[];

uniform mat4 u_view_projection;  // NEW: Need this in geometry shader
uniform float u_normal_length;
uniform bool u_show_tangents;

out vec4 g_color;

// 
void main()
{
    for (int i = 0; i < 3; i++) {
        vec3 pos = gs_in[i].world_pos;
        
        // Draw normal (cyan)
        g_color = vec4(0.0, 1.0, 1.0, 1.0);
        gl_Position = u_view_projection * vec4(pos, 1.0);  // Project start point
        EmitVertex();
        
        vec3 normal_end = pos + gs_in[i].normal * u_normal_length;
        gl_Position = u_view_projection * vec4(normal_end, 1.0);  // Project end point
        EmitVertex();
        EndPrimitive();
        
        // Draw tangent (magenta)
        if (u_show_tangents) {
            g_color = vec4(1.0, 0.0, 1.0, 1.0);
            gl_Position = u_view_projection * vec4(pos, 1.0);
            EmitVertex();
            
            vec3 tangent_end = pos + gs_in[i].tangent * u_normal_length;
            gl_Position = u_view_projection * vec4(tangent_end, 1.0);
            EmitVertex();
            EndPrimitive();
        }
    }
}


#type FRAGMENT_SHADER
#version 450 core

in vec4 g_color;
out vec4 frag_color;

// 
void main()
{
    frag_color = g_color;
    
}

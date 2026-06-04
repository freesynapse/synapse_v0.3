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

uniform samplerCube u_environment_map;

const float PI = 3.14159265359;

// 
void main()
{
    vec3 normal = normalize(v_uv);
    vec3 irradiance = vec3(0.0);

    // tangent space calc to align samples with the normal
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sample_delta = 0.025;
    float sample_count = 0.0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sample_delta) {
        for (float theta  = 0.0; theta < 0.5 * PI; theta += sample_delta) {
            // spherical to cartesian (in tangent space)
            vec3 tangent_sample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world space
            vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal; 

            irradiance += texture(u_environment_map, sample_vec).rgb * cos(theta) * sin(theta);
            sample_count++;            
        }
    }

    irradiance = PI * irradiance * (1.0 / float(sample_count));
    frag_color = vec4(irradiance, 1.0);
    
}
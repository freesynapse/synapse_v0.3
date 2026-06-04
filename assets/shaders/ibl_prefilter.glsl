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
uniform float u_roughness;

const float PI = 3.14159265359;

// --- Helper Functions for Monte Carlo Integration ---
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; 
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness*roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// 
void main()
{
    vec3 N = normalize(v_uv);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float total_weight = 0.0f;
    vec3 prefiltered_color = vec3(0.0);

    for (uint i = 0; i < SAMPLE_COUNT; i++) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, u_roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            prefiltered_color += texture(u_environment_map, L).rgb * NdotL;
            total_weight += NdotL;
        }
    }

    prefiltered_color = prefiltered_color / total_weight;
    frag_color = vec4(prefiltered_color, 1.0);
    
}

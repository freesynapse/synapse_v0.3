#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec3 a_bitangent;
layout(location = 4) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform mat3 u_normal_matrix;

out vec3 v_world_pos;
out vec2 v_uv;
out mat3 v_tbn;

void main()
{
    v_world_pos = vec3(u_model * vec4(a_position, 1.0));
    v_uv = a_uv;

    // Construct TBN Matrix for Tangent-to-World space transformation
    vec3 T = normalize(u_normal_matrix * a_tangent);
    vec3 B = normalize(u_normal_matrix * a_bitangent);
    vec3 N = normalize(u_normal_matrix * a_normal);
    v_tbn = mat3(T, B, N);

    gl_Position = u_view_projection * vec4(v_world_pos, 1.0);
}

#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 frag_color;

in vec3 v_world_pos;
in vec2 v_uv;
in mat3 v_tbn;

// Material UBO (64 bytes)
layout (std140, binding=1) uniform u_material_block {
    vec4  u_albedo_color;
    vec4  u_emissive_color;
    
    float u_roughness;
    float u_metallic;
    float u_ao;
    float u_tiling_factor;

    float u_use_albedo_map;
    float u_use_normal_map;
    float u_use_metallic_map;
    float u_use_roughness_map;
    float u_use_ao_map;
    float u_use_emissive_map;

    float _pad[2];
};

struct light_t {
    vec4 position;
    vec4 color;
    vec4 direction;
    vec4 params;
};

layout (std140, binding=2) uniform u_lighting_block {
    light_t lights[16];
    uint u_light_count;
};

// Texture Slots
layout(binding=0) uniform sampler2D u_albedo_map;
layout(binding=1) uniform sampler2D u_normal_map;
layout(binding=2) uniform sampler2D u_metallic_map;
layout(binding=3) uniform sampler2D u_roughness_map;
layout(binding=4) uniform sampler2D u_ao_map;
layout(binding=5) uniform sampler2D u_emissive_map;
layout(binding=6) uniform samplerCube u_irradiance_map;
layout(binding=7) uniform samplerCube u_prefilter_map;

uniform vec3 u_view_pos;

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

// --- PBR Functions ---

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec2 uv = v_uv * u_tiling_factor;

    // 1. Sample Textures (or use constants)    
    vec3 albedo = (u_use_albedo_map > 0.5) ? texture(u_albedo_map, uv).rgb : u_albedo_color.rgb;
    float metallic = (u_use_metallic_map > 0.5) ? texture(u_metallic_map, uv).r : u_metallic;
    float roughness = (u_use_roughness_map > 0.5) ? texture(u_roughness_map, uv).r : u_roughness;
    float ao = (u_use_ao_map > 0.5) ? texture(u_ao_map, uv).r : u_ao;
    vec3 emissive = (u_use_emissive_map > 0.5) ? texture(u_emissive_map, uv).rgb : u_emissive_color.rgb;

    // linearize colors sRGB -> linear
    albedo = pow(albedo, vec3(2.2));
    emissive = pow(emissive, vec3(2.2));
    
    // 2. Normal Mapping
    vec3 N;
    if (u_use_normal_map > 0.5) {
        N = texture(u_normal_map, uv).rgb;
        N = normalize(v_tbn * (N * 2.0 - 1.0));
    } else {
        N = normalize(v_tbn[2]); // Use vertex normal (third column of TBN)
    }

    vec3 V = normalize(u_view_pos - v_world_pos);

    // 3. Lighting Calculation
    vec3 F0 = vec3(0.04); // Default dielectric reflectance
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 radiance;
    float attenuation = 1.0;

    // int light_count = int(u_light_count);
    int light_count = 0;
    for (int i = 0; i < light_count; i++) {

        float type = lights[i].position.w;
        vec3 light_pos = lights[i].position.xyz;
        vec3 light_color = lights[i].color.rgb;
        float intensity = lights[i].color.a;
        vec3 L = vec3(0, 1, 0);

        // 0 : point light
        if (type < 0.5) {
            L = normalize(light_pos - v_world_pos);
            float distance = length(light_pos - v_world_pos);
            attenuation = 1.0 / (distance * distance);
            radiance = light_color * intensity * attenuation;
        }

        // 1 : directional light
        else if (type < 1.5) {
            L = normalize(-lights[i].direction.xyz);
            radiance = light_color * intensity;
        }

        // 2 : spotlight
        else {
            L = normalize(light_pos - v_world_pos);
            float distance = length(light_pos - v_world_pos);
            attenuation = 1.0 / (distance * distance);

            // spotlight specific
            float theta = dot(L, normalize(-lights[i].direction.xyz));
            float inner = lights[i].params.x;
            float outer = lights[i].params.y;
            float epsilon = inner - outer;
            float spot_intensity = clamp((theta - outer) / epsilon, 0.0, 1.0);

            radiance = light_color * intensity * attenuation * spot_intensity;            
        }

        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        vec3 specular = numerator / denominator;
    
        // vec3 kS = F;
        vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    }

    vec3 kS_env = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD_env = (vec3(1.0) - kS_env) * (1.0 - metallic);
    
    vec3 irradiance = texture(u_irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    vec3 R = reflect(-V, N);
    vec3 prefiltered_color = textureLod(u_prefilter_map, R, roughness * MAX_REFLECTION_LOD).rgb;
    // vec3 prefiltered_color = texture(u_prefilter_map, R).rgb;
    vec3 env_specular = prefiltered_color * kS_env;
        
    // 4. Final Color Assembly
    vec3 ambient = (kD_env * diffuse + env_specular) * ao;
    // vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo + emissive;

    // HDR Tonemapping & Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    frag_color = vec4(color, 1.0);
    
}

#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_color_texture;    // albedo.rgb + roughness
layout(binding = 1) uniform sampler2D in_normal_texture;   // normal.xyz (packed) + metalness
layout(binding = 2) uniform sampler2D in_depth_texture;

layout(binding = 4) uniform samplerCube in_envmap;
layout(binding = 5) uniform sampler2D brdf_lut;
layout(binding = 6) uniform sampler2DShadow shadow_map;

uniform uint display_step;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

// Rebuild the world-space position from the pixel's UV and its depth
vec3 world_position(vec2 uv, float depth) {
    const vec4 ndc = vec4(uv * 2.0 - 1.0, depth, 1.0);   // z is already in [0,1]: do NOT remap it
    const vec4 ws = frame.camera.inv_view_proj * ndc;
    return ws.xyz / ws.w;
}

float shadow_factor(vec3 world_pos) {
    // paste your current version here (with sun_bias and the 3x3 PCF loop)
    const vec3 biased_pos = world_pos + frame.sun_dir * frame.sun_bias;
    const vec4 clip = frame.WS_to_CS * vec4(biased_pos, 1.0);
    const vec3 ndc = clip.xyz / clip.w;
    const vec2 uv = ndc.xy * 0.5 + 0.5;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
        return 1.0;
    }
    return texture(shadow_map, vec3(uv, ndc.z));
}

void main() {
    const ivec2 px = ivec2(gl_FragCoord.xy);
    const vec2 uv = gl_FragCoord.xy / vec2(textureSize(in_depth_texture, 0));

    const vec4 albedo_tex = texelFetch(in_color_texture, px, 0);
    const vec4 norm_tex   = texelFetch(in_normal_texture, px, 0);
    const float depth     = texelFetch(in_depth_texture, px, 0).r;

    vec3 color = vec3(0.0);
    switch (display_step) {
        case 0: color = vec3(pow(depth, 0.35)); break;
        case 1: color = albedo_tex.rgb;         break;
        case 2: color = norm_tex.rgb;           break;
        case 3: color = vec3(albedo_tex.a);     break;
        case 4: color = vec3(norm_tex.a);       break;
        case 5: {
            // Reverse-Z: depth 0 is infinitely far, so this is sky (nothing to light)
            if (depth <= 0.0) {
                break;
            }

            const vec3 position   = world_position(uv, depth);
            const vec3 normal     = normalize(norm_tex.rgb * 2.0 - 1.0);   // unpack from [0,1]
            const vec3 base_color = albedo_tex.rgb;
            const float roughness = albedo_tex.a;
            const float metallic  = norm_tex.a;
            const vec3 view_dir   = normalize(frame.camera.position - position);

            vec3 acc = eval_ibl(in_envmap, brdf_lut, normal, view_dir, base_color, metallic, roughness)
                       * frame.ibl_intensity;

            acc += shadow_factor(position) * frame.sun_color
                   * eval_brdf(normal, view_dir, frame.sun_dir, base_color, metallic, roughness);

            for (uint i = 0; i != frame.point_light_count; ++i) {
                const PointLight light = point_lights[i];
                const vec3 to_light = light.position - position;   // <- your "in_position"
                const float dist = length(to_light);
                const vec3 light_vec = to_light / dist;

                const float att = attenuation(dist, light.radius);
                if (att <= 0.0) {
                    continue;
                }
                acc += eval_brdf(normal, view_dir, light_vec, base_color, metallic, roughness)
                       * att * light.color;
            }
            color = acc;
            break;
        }
    }

    out_color = vec4(color, 1.0);
}

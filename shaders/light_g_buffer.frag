#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(binding = 4) uniform samplerCube in_envmap;
layout(binding = 5) uniform sampler2D brdf_lut;
layout(binding = 6) uniform sampler2DShadow shadow_map;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform Data {
    FrameData frame;
};

#include "g_buffer_read.glsl"

// TODO: Put it in utils.glsl because it is used multiple time ?
float shadow_factor(vec3 world_pos) {
    const vec3 biased_pos = world_pos + frame.sun_dir * frame.sun_bias;

    const vec4 clip = frame.WS_to_CS * vec4(biased_pos, 1.0);
    const vec3 normaled = clip.xyz / clip.w;
    const vec2 uv = (normaled.xy + 1.0) / 2.0;

    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
        return 1.0;
    }

    const vec2 texel = 1.0 / vec2(textureSize(shadow_map, 0));

    float shadow = 0.0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            shadow += texture(shadow_map, vec3(uv + vec2(i, j) * texel, normaled.z));
        }
    }

    return shadow / 9.0;
}

void main() {
    const GBufferSample g = read_g_buffer(ivec2(gl_FragCoord.xy));

    if (g.depth <= 0.0) {
        const vec2 uv = gl_FragCoord.xy / vec2(textureSize(in_depth_texture, 0));
        const vec3 sky_dir = normalize(unproject(uv, 0.001, frame.camera.inv_view_proj) - frame.camera.position);
        out_color = vec4(texture(in_envmap, sky_dir).rgb * frame.ibl_intensity, 0.0);
        return;
    }

    const vec3 view_dir = normalize(frame.camera.position - g.position);

    vec3 acc = eval_ibl(in_envmap, brdf_lut, g.normal, view_dir, g.base_color, g.metallic, g.roughness)
               * frame.ibl_intensity;

    acc += shadow_factor(g.position) * frame.sun_color
           * eval_brdf(g.normal, view_dir, frame.sun_dir, g.base_color, g.metallic, g.roughness);

    out_color = vec4(acc, 1.0);
}

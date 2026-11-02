#version 450

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_color_texture;    // albedo + roughness
layout(binding = 1) uniform sampler2D in_normal_texture;   // normal + metalness
layout(binding = 2) uniform sampler2D in_depth_texture;    // depth buffer

uniform uint display_step;

void main() {
    const ivec2 px = ivec2(gl_FragCoord.xy);

    const vec4 albedo_tex = texelFetch(in_color_texture, px, 0);
    const vec4 norm_tex   = texelFetch(in_normal_texture, px, 0);
    const float depth     = texelFetch(in_depth_texture, px, 0).r;

    vec3 color = vec3(0.0);
    switch (display_step) {
        case 0: color = vec3(pow(depth, 0.35)); break;   // depth
        case 1: color = albedo_tex.rgb;         break;   // albedo
        case 2: color = norm_tex.rgb;           break;   // normal (still packed 0..1)
        case 3: color = vec3(albedo_tex.a);     break;   // roughness
        case 4: color = vec3(norm_tex.a);       break;   // metalness
    }

    out_color = vec4(color, 1.0);
}
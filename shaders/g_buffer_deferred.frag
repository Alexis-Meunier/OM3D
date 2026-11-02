#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;
layout(binding = 2) uniform sampler2D in_metal_rough;

uniform vec3 base_color_factor;
uniform vec2 metal_rough_factor;
uniform float alpha_cutoff;

void main() {
    const vec4 albedo_tex = texture(in_texture, in_uv);

#ifdef ALPHA_TEST
    if (albedo_tex.a <= alpha_cutoff) {
        discard;
    }
#endif

    const vec3 nm = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
    const vec3 normal = normalize(nm.x * in_tangent + nm.y * in_bitangent + nm.z * in_normal);

    const vec4 mr = texture(in_metal_rough, in_uv);
    const float roughness = mr.g * metal_rough_factor.y;
    const float metallic = mr.b * metal_rough_factor.x;

    out_color = vec4(in_color * albedo_tex.rgb * base_color_factor, roughness);
    out_normal = vec4(normal * 0.5 + 0.5, metallic);
}
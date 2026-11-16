layout(binding = 0) uniform sampler2D in_color_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;
layout(binding = 2) uniform sampler2D in_depth_texture;

struct GBufferSample {
    vec3 position;
    vec3 normal;
    vec3 base_color;
    float roughness;
    float metallic;
    float depth;
};

GBufferSample read_g_buffer(ivec2 px) {
    GBufferSample s;

    s.depth = texelFetch(in_depth_texture, px, 0).r;

    const vec4 albedo_tex = texelFetch(in_color_texture, px, 0);
    const vec4 norm_tex = texelFetch(in_normal_texture, px, 0);

    const vec2 uv = (vec2(px) + 0.5) / vec2(textureSize(in_depth_texture, 0));
    s.position = unproject(uv, s.depth, frame.camera.inv_view_proj);

    s.normal = normalize(norm_tex.rgb * 2.0 - 1.0);
    s.base_color = albedo_tex.rgb;
    s.roughness = albedo_tex.a;
    s.metallic = norm_tex.a;

    return s;
}

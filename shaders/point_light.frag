#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform Data {
    FrameData frame;
};

#include "g_buffer_read.glsl"

uniform vec3 light_position;
uniform float light_radius;
uniform vec3 light_color;

void main() {
    const GBufferSample g = read_g_buffer(ivec2(gl_FragCoord.xy));

    const vec3 to_light = light_position - g.position;
    const float dist = length(to_light);

    const float att = attenuation(dist, light_radius);
    if (att <= 0.0) {
        discard;
    }

    const vec3 view_dir = normalize(frame.camera.position - g.position);
    const vec3 light_vec = to_light / dist;

    out_color = vec4(eval_brdf(g.normal, view_dir, light_vec, g.base_color, g.metallic, g.roughness)
                     * att * light_color, 0.0);
}

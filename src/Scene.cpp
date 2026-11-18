#include "Scene.h"

#include <iostream>
#include <TypedBuffer.h>

#include <shader_structs.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>

namespace OM3D {

extern bool g_buffer_enabled;

Scene::Scene() {
    _sky_material.set_program(Program::from_files("sky.frag", "screen.vert"));
    _sky_material.set_depth_test_mode(DepthTestMode::None);

    _envmap = std::make_shared<Texture>(Texture::empty_cubemap(4, ImageFormat::RGBA8_UNORM));
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_light(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Span<const SceneObject> Scene::objects() const {
    return _objects;
}

Span<const PointLight> Scene::point_lights() const {
    return _point_lights;
}

Camera& Scene::camera() {
    return _camera;
}

const Camera& Scene::camera() const {
    return _camera;
}

void Scene::set_envmap(std::shared_ptr<Texture> env) {
    _envmap = std::move(env);
}

void Scene::set_ibl_intensity(float intensity) {
    _ibl_intensity = intensity;
}

void Scene::set_sun_bias(const float& sun_bias)
{
    _sun_bias = sun_bias;
}

void Scene::set_debug_state(const DebugState& state)
{
    _state = state;
}

void Scene::set_sun(float altitude, float azimuth, glm::vec3 color) {
    // Convert from degrees to radians
    const float alt = glm::radians(altitude);
    const float azi = glm::radians(azimuth);
    // Convert from polar to cartesian
    sun_direction = glm::vec3(sin(azi) * cos(alt), sin(alt), cos(azi) * cos(alt));
    _sun_color = color;
}

float average(float min, float max)
{
    return min + (max - min) / 2.0f;
}

glm::vec3 get_sun_position(const std::vector<SceneObject>& objects, const glm::vec3& sun_dir, float& radius)
{
    glm::vec3 max_pos = objects[0].getBbox().get_bounds()[0];
    glm::vec3 min_pos = objects[0].getBbox().get_bounds()[0];

    for (const auto& obj : objects)
    {
        auto bbox = obj.getBbox();
        for (const auto& bounds : bbox.get_bounds())
        {
            if (bounds.x > max_pos.x)
                max_pos.x = bounds.x;
            else if (bounds.x < min_pos.x)
                min_pos.x = bounds.x;

            if (bounds.y > max_pos.y)
                max_pos.y = bounds.y;
            else if (bounds.y < min_pos.y)
                min_pos.y = bounds.y;

            if (bounds.z > max_pos.z)
                max_pos.z = bounds.z;
            else if (bounds.z < min_pos.z)
                min_pos.z = bounds.z;
        }
    }

    glm::vec3 middle = glm::vec3(average(min_pos.x, max_pos.x), average(min_pos.y, max_pos.y), average(min_pos.z, max_pos.z));
    glm::vec3 radius_vec = middle - glm::vec3(min_pos.x, max_pos.y, max_pos.z);
    radius = sqrt(radius_vec.x * radius_vec.x + radius_vec.y * radius_vec.y + radius_vec.z * radius_vec.z);

    return middle + glm::normalize(sun_dir) * radius;
}

void Scene::update_shadow_matrix() {
    glm::vec3 mn(std::numeric_limits<float>::max());
    glm::vec3 mx(std::numeric_limits<float>::lowest());
    for (const SceneObject& obj : _objects) {
        for (const glm::vec3& c : obj.getBbox().get_bounds()) {
            const glm::vec3 w = glm::vec3(obj.transform() * glm::vec4(c, 1.0f));
            mn = glm::min(mn, w);
            mx = glm::max(mx, w);
        }
    }
    const glm::vec3 center = (mn + mx) * 0.5f;
    const float radius = glm::length(mx - mn) * 0.5f;

    const glm::vec3 dir = glm::normalize(sun_direction);
    const glm::vec3 up = std::abs(dir.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    const glm::mat4 view = glm::lookAt(center + dir * radius, center, up);
    const glm::mat4 proj = Camera::orthographic(-radius, radius, -radius, radius, 0.0f, 2.0f * radius);
    shadow_view_proj = proj * view;
}

void Scene::render_shadow_map()
{
    update_shadow_matrix();

    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = shadow_view_proj;
        mapping[0].camera.inv_view_proj = glm::inverse(shadow_view_proj);
        mapping[0].camera.position = _camera.position();
        mapping[0].WS_to_CS = shadow_view_proj;
        mapping[0].sun_bias = _sun_bias;
        mapping[0].point_light_count = 0;
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(sun_direction);
        mapping[0].ibl_intensity = _ibl_intensity;
    }
    buffer.bind(BufferUsage::Uniform, 0);

    for (const SceneObject& obj : _objects)
    {
        if (obj.material().is_opaque())
            obj.render(SHADOW_PASS);
    }
}

bool should_display(const PointLight& light, const Frustum& f, const glm::vec3& cam_pos)
{
    glm::vec3 c = light.position() - cam_pos;

    const glm::vec3 planes[5] =
    {
        f._near_normal, f._top_normal, f._bottom_normal,
        f._right_normal, f._left_normal
    };

    for (const glm::vec3 n : planes)
    {
        if (glm::dot(c, n) < -light.radius())
        {
            return false;
        }
    }

    return true;
}

TypedBuffer<shader::FrameData> Scene::make_frame_data(u32 point_light_count) const {
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(_camera.view_proj_matrix());
        mapping[0].camera.position = _camera.position();
        mapping[0].WS_to_CS = shadow_view_proj;
        mapping[0].sun_bias = _sun_bias;
        mapping[0].point_light_count = point_light_count;
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(sun_direction);
        mapping[0].ibl_intensity = _ibl_intensity;
    }
    return buffer;
}

static void restore_default_render_state() {
    ENABLE_BACKCULL_CCW;
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

TypedBuffer<shader::PointLight> Scene::make_point_light_buffer(const Frustum& frustum, u32& count) const {
    const glm::vec3 cam_pos = _camera.position();
    count = 0;

    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for(const PointLight& light : _point_lights) {
            if(!should_display(light, frustum, cam_pos)) {
                continue;
            }
            mapping[count++] = {
                light.position(),
                light.radius(),
                light.color(),
                0.0f
            };
        }
    }
    return light_buffer;
}

void Scene::render_transparent() const {
    const Frustum frustum = _camera.build_frustum();
    const glm::vec3 cam_pos = _camera.position();

    u32 nb_light = 0;
    const TypedBuffer<shader::PointLight> light_buffer = make_point_light_buffer(frustum, nb_light);
    light_buffer.bind(BufferUsage::Storage, 1);

    const TypedBuffer<shader::FrameData> buffer = make_frame_data(nb_light);
    buffer.bind(BufferUsage::Uniform, 0);

    DEBUG_ASSERT(_envmap && !_envmap->is_null());
    _envmap->bind(4);
    brdf_lut().bind(5);

    for(const SceneObject& obj : _objects) {
        if(obj.material().is_opaque() || !obj.getBbox().should_draw(frustum, cam_pos, obj.transform())) {
            continue;
        }
        obj.render(MAIN);
    }

    restore_default_render_state();
}

void Scene::render_sun_light(const Material& sun_material) const {
    const TypedBuffer<shader::FrameData> buffer = make_frame_data(0);
    buffer.bind(BufferUsage::Uniform, 0);

    DEBUG_ASSERT(_envmap && !_envmap->is_null());
    _envmap->bind(4);
    brdf_lut().bind(5);

    sun_material.bind();
    draw_full_screen_triangle();
}

void Scene::render_point_lights(const Material& light_material, const StaticMesh& sphere) const {
    const TypedBuffer<shader::FrameData> buffer = make_frame_data(0);
    buffer.bind(BufferUsage::Uniform, 0);

    const Frustum frustum = _camera.build_frustum();
    const glm::vec3 cam_pos = _camera.position();

    for(const PointLight& light : _point_lights) {
        if(!should_display(light, frustum, cam_pos)) {
            continue;
        }

        const float scale = light.radius() * light_volume_margin;
        const glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), light.position()), glm::vec3(scale));

        light_material.set_uniform(HASH("model"), model);
        light_material.set_uniform(HASH("light_position"), light.position());
        light_material.set_uniform(HASH("light_radius"), light.radius());
        light_material.set_uniform(HASH("light_color"), light.color());
        light_material.bind();
        sphere.draw();
    }

    restore_default_render_state();
}

void Scene::render_prepass(const RenderPass& state) {
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(_camera.view_proj_matrix());
        mapping[0].camera.position = _camera.position();
        mapping[0].WS_to_CS = shadow_view_proj;
        mapping[0].sun_bias = _sun_bias;
        mapping[0].point_light_count = 0;
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(sun_direction);
        mapping[0].ibl_intensity = _ibl_intensity;
    }
    buffer.bind(BufferUsage::Uniform, 0);
 
    {
        const Frustum frustum = _camera.build_frustum();
        const glm::vec3 cam_pos = _camera.position();

        for(const SceneObject& obj : _objects) {
            if(!obj.getBbox().should_draw(frustum, cam_pos, obj.transform())
                || !obj.material().is_opaque()
                || (state == Z_PREPASS && !obj.can_use_prepass())) {
                continue;
            }
    
            obj.render(state);
        }
    }
}

void Scene::render() const {
    const Frustum frustum = _camera.build_frustum();
    const glm::vec3 cam_pos = _camera.position();
    u32 nb_light = 0;
    const TypedBuffer<shader::PointLight> light_buffer = make_point_light_buffer(frustum, nb_light);
    light_buffer.bind(BufferUsage::Storage, 1);

    // Fill and bind frame data buffer
    const TypedBuffer<shader::FrameData> buffer = make_frame_data(nb_light);
    buffer.bind(BufferUsage::Uniform, 0);

    // Bind envmap
    DEBUG_ASSERT(_envmap && !_envmap->is_null());
    _envmap->bind(4);

    // Bind brdf lut needed for lighting to scene rendering shaders
    brdf_lut().bind(5);

    // Render the sky
    _sky_material.bind();
    _sky_material.set_uniform(HASH("intensity"), _ibl_intensity);
    draw_full_screen_triangle();

    // Render every object
    {
        // Opaque first
        for(const SceneObject& obj : _objects) {
            if (!obj.getBbox().should_draw(frustum, cam_pos, obj.transform()))
                continue;

            if(obj.material().is_opaque()) {
                obj.render(g_buffer_enabled ? G_BUFFER_RENDER : MAIN);
            }
        }

        // Transparent after
        for(const SceneObject& obj : _objects) {
            if (!obj.getBbox().should_draw(frustum, cam_pos, obj.transform()))
                continue;

            if(!obj.material().is_opaque()) {
                obj.render(g_buffer_enabled ? G_BUFFER_RENDER : MAIN);
            }
        }
    }

}

}

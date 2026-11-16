#ifndef SCENE_H
#define SCENE_H

#include <Bbox.h>
#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>
#include <TypedBuffer.h>
#include <shader_structs.h>

#include <vector>
#include <memory>

namespace OM3D {

class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void render_prepass(const RenderPass& state);
        void render_shadow_map();

        void render() const;
        void render_sun_light(const Material& sun_material) const;
        void render_point_lights(const Material& light_material, const StaticMesh& sphere) const;
        void render_transparent() const;

        static constexpr float light_volume_margin = 1.1f;

        void add_object(SceneObject obj);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera& camera();
        const Camera& camera() const;

        void set_envmap(std::shared_ptr<Texture> env);
        void set_ibl_intensity(float intensity);

        void set_debug_state(const DebugState& state);

        void set_sun(float altitude, float azimuth, glm::vec3 color = glm::vec3(1.0f));
        void set_sun_bias(const float& sun_bias);
        void update_shadow_matrix();
        
        glm::vec3 sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
    private:
        TypedBuffer<shader::FrameData> make_frame_data(u32 point_light_count) const;
        TypedBuffer<shader::PointLight> make_point_light_buffer(const Frustum& frustum, u32& count) const;

        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;

        glm::vec3 _sun_color = glm::vec3(1.0f);

        std::shared_ptr<Texture> _envmap;
        float _ibl_intensity = 1.0f;
        Material _sky_material;

        DebugState _state = ALBEDO;

        Camera _camera;
        glm::mat4 shadow_view_proj = glm::identity<glm::mat4>();

        float _sun_bias = 0.0f;
};

}

#endif // SCENE_H

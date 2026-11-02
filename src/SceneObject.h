#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <Camera.h>
#include <StaticMesh.h>
#include <Material.h>

#include <memory>

#include <glm/matrix.hpp>

namespace OM3D {

enum DebugState {
    DEPTH = 0,
    ALBEDO = 1,
    NORMAL = 2,
    ROUGHNESS = 3,
    METALLICITY = 4,
};

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const RenderPass& state = MAIN) const;

        // void render_prepass() const;
        // void render_shadow_map() const;
        // void render_g_buffer() const;

        const Material& material() const;

        bool can_use_prepass() const;

        Bbox getBbox() const;

        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;

    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
};

}

#endif // SCENEOBJECT_H
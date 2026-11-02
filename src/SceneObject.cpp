#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

extern bool enable_prepass;
extern bool g_buffer_enabled;

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::render(const RenderPass& state) const {
    if(!_material || !_mesh) return;

    _material->set_rendering_state((state == MAIN && g_buffer_enabled) ? G_BUFFER_RENDER : state);
    if (state < 3)
        _material->set_depth_test_mode((enable_prepass && can_use_prepass()) ? DepthTestMode::Equal : DepthTestMode::Standard);
    else
        _material->set_depth_test_mode(DepthTestMode::Equal);

    _material->set_uniform(HASH("model"), transform());
    _material->bind();
    _mesh->draw();
}

// void SceneObject::render_prepass() const {
//     if(!_mesh) {
//         return;
//     }

//     _material->set_rendering_state(Z_PREPASS);
//     _material->set_depth_test_mode(DepthTestMode::Equal);
//     _material->set_uniform(HASH("model"), transform());
//     _material->bind();
//     _mesh->draw();
// }

// void SceneObject::render_shadow_map() const
// {
//     if(!_material || !_mesh) {
//         return;
//     }

//     _material->set_rendering_state(SHADOW_PASS);
//     _material->set_depth_test_mode(DepthTestMode::Standard);
//     _material->set_uniform(HASH("model"), transform());
//     _material->bind();
//     _mesh->draw();
// }

// void SceneObject::render_g_buffer() const
// {
//     if(!_material || !_mesh) {
//         return;
//     }

//     _material->set_rendering_state(G_BUFFER_DEFERRED);
//      _material->set_depth_test_mode(
//         (enable_prepass && can_use_prepass()) ? DepthTestMode::Equal : DepthTestMode::Standard);
//     _material->set_uniform(HASH("model"), transform());
//     _material->bind();
//     _mesh->draw();
// }

bool SceneObject::can_use_prepass() const {
    return _mesh && _material && _material->is_opaque() && !_material->is_alpha_tested();
}

Bbox SceneObject::getBbox() const
{
    return _mesh->getBbox();
}

const Material& SceneObject::material() const {
    DEBUG_ASSERT(_material);
    return *_material;
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}

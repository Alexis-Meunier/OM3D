#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::render() const {
    if(!_material || !_mesh) {
        return;
    }

    if (!_material->is_opaque())
        _material->set_depth_test_mode(DepthTestMode::Standard);
    _material->set_uniform(HASH("model"), transform());
    _material->bind();
    _mesh->draw();
}

void SceneObject::render_shadow_map(const Material& shadow_material) const
{
    if(!_material || !_mesh) {
        return;
    }

    shadow_material.set_uniform(HASH("model"), transform());
    shadow_material.bind();
    _mesh->draw();
}

void SceneObject::render_prepass(const Material& prepass_material) const {
    if(!_mesh) {
        return;
    }

    _material->set_depth_test_mode(DepthTestMode::Equal);
    prepass_material.set_uniform(HASH("model"), transform());
    prepass_material.bind();
    _mesh->draw();
}

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

#include "Material.h"

#include <glad/gl.h>

#include <algorithm>

namespace OM3D {

Material::Material() {
}

void Material::set_program(std::shared_ptr<Program> prog) {
    _programs[0] = std::move(prog);
}

void Material::set_blend_mode(BlendMode blend) {
    _blend_mode = blend;
}

void Material::set_depth_test_mode(DepthTestMode depth) {
    _depth_test_mode = depth;
}

void Material::set_double_sided(bool double_sided) {
    _double_sided = double_sided;
}

void Material::set_depth_write(bool depth_write) {
    _depth_write = depth_write;
}

void Material::set_cull_front_faces(bool cull_front) {
    _cull_front = cull_front;
}

void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex) {
    if(const auto it = std::find_if(_textures.begin(), _textures.end(), [&](const auto& t) { return t.first == slot; }); it != _textures.end()) {
        it->second = std::move(tex);
    } else {
        _textures.emplace_back(slot, std::move(tex));
    }
}

bool Material::is_opaque() const {
    return _blend_mode == BlendMode::None;
}

void Material::set_stored_uniform(u32 name_hash, UniformValue value) {
    for(auto& [h, v] : _uniforms) {
        if(h == name_hash) {
            v = value;
            return;
        }
    }
    _uniforms.emplace_back(name_hash, std::move(value));
}

void Material::set_rendering_state(const RenderPass& pass)
{
    _render_state = pass;
}

void Material::bind() const {
    switch(_blend_mode) {
        case BlendMode::None:
            ENABLE_BACKCULL_CCW;
            glDisable(GL_BLEND);
        break;

        case BlendMode::Alpha:
            DISABLE_FACECULL;
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;

        case BlendMode::Additive:
            ENABLE_BACKCULL_CCW;
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        break;
    }

    if(_cull_front) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
    }

    glDepthMask(_depth_write ? GL_TRUE : GL_FALSE);

    switch(_depth_test_mode) {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
        break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
        break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
        break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
        break;
    }

    for(const auto& texture : _textures) {
        texture.second->bind(texture.first);
    }

    for(const auto& [h, v] : _uniforms) {
        _programs[_render_state]->set_uniform(h, v);
    }

    _programs[_render_state]->bind();
}

Material Material::textured_pbr_material(bool alpha_test) {
    Material material;
    material._alpha_test = alpha_test;

    std::vector<std::string> defines;
    if(alpha_test) {
        defines.emplace_back("ALPHA_TEST");
    }

    material._programs[MAIN] = Program::from_files("lit.frag", "basic.vert", defines);
    material._programs[Z_PREPASS] = Program::from_files("z_prepass.frag", "basic.vert", defines);
    material._programs[SHADOW_PASS] = Program::from_files("shadow.frag", "basic.vert", defines);
    material._programs[G_BUFFER_DEFERRED] = Program::from_files("g_buffer_deferred.frag", "basic.vert", defines);
    material._programs[G_BUFFER_RENDER] = Program::from_files("g_buffer_render.frag", "basic.vert", defines);

    material.set_texture(0u, default_white_texture());
    material.set_texture(1u, default_normal_texture());
    material.set_texture(2u, default_metal_rough_texture());
    material.set_texture(3u, default_white_texture());

    return material;
}


}

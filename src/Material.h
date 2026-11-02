#ifndef MATERIAL_H
#define MATERIAL_H

#include <Program.h>
#include <Texture.h>

#include <memory>
#include <vector>

namespace OM3D {

enum class BlendMode {
    None,
    Alpha,
};

enum class DepthTestMode {
    Standard,
    Reversed,
    Equal,
    None
};

enum RenderPass {
    MAIN = 0,
    G_BUFFER_DEFERRED = 1,
    G_BUFFER_RENDER = 2,
    Z_PREPASS = 3,
    SHADOW_PASS = 4,
};

class Material {

    public:
        Material();

        void set_program(std::shared_ptr<Program> prog);
        void set_blend_mode(BlendMode blend);
        void set_depth_test_mode(DepthTestMode depth);
        void set_double_sided(bool double_sided);
        void set_texture(u32 slot, std::shared_ptr<Texture> tex);

        bool is_opaque() const;

        // Uniform will be stored inside the material and reset every time its bound
        void set_stored_uniform(u32 name_hash, UniformValue value);

        // Uniform is set immediately and might get overriden by 'set_uniform' called on OTHER materials
        template<typename... Args>
        void set_uniform(Args&&... args) const {
            _programs[_render_state]->set_uniform(FWD(args)...);
        }

        void bind() const;

        static Material textured_pbr_material(bool alpha_test = false);
        void set_alpha_tested(bool v) { _alpha_test = v; }
        bool is_alpha_tested() const { return _alpha_test; }

        void set_rendering_state(const RenderPass& pass);

    private:
        std::shared_ptr<Program> _programs[5];
        std::vector<std::pair<u32, std::shared_ptr<Texture>>> _textures;
        std::vector<std::pair<u32, UniformValue>> _uniforms;

        BlendMode _blend_mode = BlendMode::None;
        DepthTestMode _depth_test_mode = DepthTestMode::Standard;
        bool _double_sided = false;
        bool _alpha_test = false;

        RenderPass _render_state = MAIN;
};

}

#endif // MATERIAL_H

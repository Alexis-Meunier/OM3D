#ifndef BBOX_H
#define BBOX_H

#include <Camera.h>
#include <glad/gl.h>
#include <Vertex.h>

#include <iostream>
#include <vector>

namespace OM3D {

class Bbox {
    public:
        Bbox() = default;
        Bbox(Bbox&&) = default;
        Bbox& operator=(Bbox&&) = default;
        Bbox(Bbox&) = default;
        Bbox(const std::vector<Vertex>& vertices);

        bool should_draw(const Frustum& frustum, const glm::vec3& cam_pos, const glm::mat4& model) const;
        std::vector<glm::vec3> get_bounds() const;

        friend std::ostream& operator<<(const Bbox& bbox, std::ostream& os);

    private:
        std::vector<glm::vec3> _bounds;
        glm::vec3 _center;
};

}

#endif // BBOX_H
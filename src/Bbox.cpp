#include "Bbox.h"

namespace OM3D {

// TODO: Add it somewhere global
float dot(const glm::vec3& v1, const glm::vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Bbox::Bbox(const std::vector<Vertex>& vertices)
{
    assert(!vertices.empty() && "Trying to compute Bbox for an empty mesh");

    glm::vec3 max_pos = vertices[0].position;    
    glm::vec3 min_pos = vertices[0].position;    
    for (const auto& vertex : vertices)
    {
        if (vertex.position.x > max_pos.x)
            max_pos.x = vertex.position.x;
        else if (vertex.position.x < min_pos.x)
            min_pos.x = vertex.position.x;

        if (vertex.position.y > max_pos.y)
            max_pos.y = vertex.position.y;
        else if (vertex.position.y < min_pos.y)
            min_pos.y = vertex.position.y;

        if (vertex.position.z > max_pos.z)
            max_pos.z = vertex.position.z;
        else if (vertex.position.z < min_pos.z)
            min_pos.z = vertex.position.z;
    }

    _bounds = {
        // TOP plane
        {min_pos.x, max_pos.y, max_pos.z},
        {min_pos.x, max_pos.y, min_pos.z},
        {max_pos.x, max_pos.y, min_pos.z},
        {max_pos.x, max_pos.y, max_pos.z},
        // BOT plane
        {min_pos.x, min_pos.y, max_pos.z},
        {min_pos.x, min_pos.y, min_pos.z},
        {max_pos.x, min_pos.y, min_pos.z},
        {max_pos.x, min_pos.y, max_pos.z},
    };
    // In case of overflows
    _center = min_pos + (max_pos - min_pos) / 2.0f;
}

bool Bbox::should_draw(const Frustum& f, const glm::vec3& cam_pos, const glm::mat4& model) const
{
    glm::vec3 corners[8];
    for (size_t i = 0; i < 8; ++i)
    {
        corners[i] = glm::vec3(model * glm::vec4(_bounds[i], 1.0f)) - cam_pos;
    }

    const glm::vec3 planes[5] =
    {
        f._near_normal, f._top_normal, f._bottom_normal,
        f._right_normal, f._left_normal
    };

    for (const glm::vec3 n : planes)
    {
        bool all_outside = true;
        for (const glm::vec3& c : corners)
        {
            if (glm::dot(c, n) >= 0.0f)
            {
                all_outside = false;
                break;
            }
        }
        if (all_outside)
        {
            return false;
        }
    }
    return true;
}

std::vector<glm::vec3> Bbox::get_bounds() const
{
    return _bounds;
}

}

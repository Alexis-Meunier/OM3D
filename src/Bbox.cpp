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

bool Bbox::should_draw(const Camera& camera) const
{
    const glm::vec3 cam_pos = camera.position();
    const Frustum frustum = camera.build_frustum();

    std::vector<glm::vec3> normals = {
        frustum._near_normal,
        frustum._top_normal,
        frustum._bottom_normal,
        frustum._right_normal,
        frustum._left_normal
    };

    // For every normal
    for (const auto& normal : normals)
    {
        bool all_outside = true;
        // If there is at least one of the bbox bound in it should be drawn
        for (const auto& vertex : _bounds)
        {
            if (dot(vertex - cam_pos, normal) >= 0)
            {
                all_outside = false;
                break;
            }
        }
        // If all bounds are outside, it is out of view
        if (all_outside)
            return false;
    }
    return true;    
}

}

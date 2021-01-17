#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace nevk
{
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};
} // namespace nevk

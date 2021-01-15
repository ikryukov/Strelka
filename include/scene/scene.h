#pragma once

#include <vector>
#include <cstdint>
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace nevk
{
struct Vertex
{
    //glm::vec4 pos;
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec2 uv;
};

struct Mesh
{
    uint32_t mIndex; // Index of 1st index in index buffer
    uint32_t mCount; // amount of indices in mesh
};

struct Material
{
    glm::vec4 color;
};

struct Instance
{
    glm::mat4 transform;
    uint32_t mMeshId;
    uint32_t mMaterialId;
};

class Scene
{
private:
    //    Camera camera;
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;

public:
    Scene()
    {
    }
    ~Scene()
    {
    }
    void update_mesh();

    /// <summary>
    /// Create Mesh geometry
    /// </summary>
    /// <param name="vb">Vertices</param>
    /// <param name="ib">Indices</param>
    /// <returns>Mesh id in scene</returns>
    uint32_t createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib);
    /// <summary>
    /// Creates Instance
    /// </summary>
    /// <param name="meshId">valid mesh id</param>
    /// <param name="materialId">valid material id</param>
    /// <param name="transform">transform</param>
    /// <returns>Instance id in scene</returns>
    uint32_t createInstance(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform);
    //void add(); // instance, mesh, material
    //void remove(); // instance, mesh, material
    uint32_t createMaterial(const glm::vec4& color);
    //    void update_camera();
};
} // namespace nevk

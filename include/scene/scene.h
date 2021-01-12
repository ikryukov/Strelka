#pragma once

#include <vector>
#include <stack>
#include <set>
#include <cstdint>
#include "glm-wrapper.hpp"
#include "camera.h"


namespace nevk
{

struct Vertex
{
    glm::float4 pos;
    glm::float3 normal;
    glm::float2 uv; // hold the texture coordinate

    /*
     * A vertex should really only be considered equal
     * if both the position and the texture coordinate are the same.
     * This will cause our vertex class to only be unique
     * when one or more of its properties are different in comparison to another vertex.
     */
    bool operator==(const nevk::Vertex& other) const;
};

struct Mesh
{
    uint32_t mIndex; // Index of 1st index in index buffer
    uint32_t mCount; // amount of indices in mesh
};

struct MeshInstance
{
    /*
    * Defining an object that can hold a definition of which mesh and texture to use
    * and offer a way to update and fetch its transform matrix.
    * We need the transform matrix to know where and how the mesh should be positioned in the 3d world
    */
    const glm::mat4 identity;
    glm::float3 position;
    glm::float3 scale;
    glm::float3 rotationAxis;
    float rotationDegrees;
    glm::mat4 transformMatrix;

    explicit MeshInstance(const glm::float3& position = glm::float3{ 0.0f, 0.0f, 0.0f },
                          const glm::float3& scale = glm::float3{ 1.0f, 1.0f, 1.0f },
                          const glm::float3& rotationAxis = glm::float3{ 0.0f, 1.0f, 0.0f },
                          const float& rotationDegrees = 0.0f)
        : identity(glm::mat4{ 1.0f }), // create the identity matrix needed for the subsequent matrix operations
          position(position),
          scale(scale),
          rotationAxis(rotationAxis),
          rotationDegrees(rotationDegrees),
          transformMatrix(identity){};


    void rotateBy(const float& degrees)
    { // функция поворота экземпляра
        rotationDegrees += degrees;

        if (rotationDegrees > 360.0f)
        {
            rotationDegrees -= 360.0f;
        }
        else if (rotationDegrees < -360.0f)
        {
            rotationDegrees += 360.0f;
        }
    }

    void init_rotateBy(const float& degrees); // функция поворота экземпляра

    [[nodiscard]] glm::mat4 getTransformMatrix() const;
};

//////////////////////////////////////////////////

struct Material
{
    glm::float4 color;
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
    Camera mCamera;
    std::stack<uint32_t> mDelInstances;
    std::stack<uint32_t> mDelMesh;
    std::stack<uint32_t> mDelMaterial;

public:
    bool fr_mod;

    std::set<uint32_t> mDirtyInstances;

    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;

    Scene() = default;

    ~Scene() = default;

    Camera& getCamera()
    {
        return mCamera;
    }
    //This is where the scene will perform any of its per frame logical operations
    //and is supplied with the delta for the current frame loop.
    // void update_scene(const float& delta) = 0;

    //    void update_camera();

    /// <summary>
    /// Transform matrix
    /// </summary>
    /// <returns>Nothing</returns>
    static glm::mat4 createMeshTransform();
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
    /// <summary>
    /// Creates Material
    /// </summary>
    /// <param name="color">Color</param>
    /// <returns>Nothing</returns>
    void createMaterial(const glm::float4& color);
    /// <summary>
    /// Removes instance/mesh/material
    /// </summary>
    /// <param name="meshId">valid mesh id</param>
    /// <param name="materialId">valid material id</param>
    /// <param name="instId">valid instance id</param>
    /// <returns>Nothing</returns>
    void removeInstance(uint32_t instId);
    void removeMesh(uint32_t meshId);
    void removeMaterial(uint32_t materialId);
    /// <summary>
    /// Updates Instance matrix(transform)
    /// </summary>
    /// <param name="instId">valid instance id</param>
    /// <param name="newTransform">new transformation matrix</param>
    /// <returns>Nothing</returns>
    void updateInstanceTransform(uint32_t instId, glm::mat4 newTransform);
    /// <summary>
    /// Changes status of scene and cleans up mDirty* sets
    /// </summary>
    /// <returns>Nothing</returns>
    void beginFrame();
    /// <summary>
    /// Changes status of scene
    /// </summary>
    /// <returns>Nothing</returns>
    void endFrame();
};
} // namespace nevk

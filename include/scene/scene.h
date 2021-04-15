#pragma once

#include <vector>
#include <stack>
#include <set>
#include <cstdint>
#include "glm-wrapper.hpp"
#include "camera.h"


namespace nevk
{

struct Mesh
{
    uint32_t mIndex; // Index of 1st index in index buffer
    uint32_t mCount; // amount of indices in mesh
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

    bool FrMod{};

    std::set<uint32_t> mDirtyInstances;

public:
    struct Vertex
    {
        glm::float3 pos;
        uint32_t normal;
        uint32_t uv;
        uint16_t materialId;
    };

    struct Material
    {
        glm::float4 ambient; // Ka
        glm::float4 diffuse; // Kd
        glm::float4 specular; // Ks
        glm::float4 emissive; // Ke
        glm::float4 transparency; //  d 1 -- прозрачность/непрозрачность
        float opticalDensity; // Ni
        float shininess; // Ns 16 --  блеск материала
        uint32_t illum; // illum 2 -- модель освещения
        uint32_t texDiffuseId; // map_diffuse
        uint32_t texAmbientId; // map_ambient
        uint32_t texSpecularId; // map_specular
        uint32_t texNormalId; // map_normal - map_Bump
        uint32_t pad;
    };


    glm::float4 mLightDirection;


    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;

    Scene() = default;

    ~Scene() = default;

    std::vector<Vertex>& getVertices()
    {
        return mVertices;
    }
    std::vector<uint32_t>& getIndices()
    {
        return mIndices;
    }

    std::vector<Material>& getMaterials()
    {
        return mMaterials;
    }

    Camera& getCamera()
    {
        return mCamera;
    }

    void updateCameraParams(int width, int height)
    {
        mCamera.setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
    }
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
    uint32_t createInstance(uint32_t meshId, uint32_t materialId, const glm::mat4& transform);
    /// <summary>
    /// Creates Material
    /// </summary>
    /// <param name="color">Color</param>
    /// <returns>Nothing</returns>
    uint32_t createMaterial(const glm::float4& ambient,
                            const glm::float4& diffuse,
                            const glm::float4& specular,
                            const glm::float4& emissive,
                            const glm::float4& transparency,
                            float opticalDensity,
                            float shininess,
                            uint32_t illum,
                            uint32_t texAmbientId,
                            uint32_t texDiffuseId,
                            uint32_t texSpeculaId,
                            uint32_t texNormalId);
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
    /// Get set of DirtyInstances
    /// </summary>
    /// <returns>Set of instances</returns>
    std::set<uint32_t> getDirtyInstances();
    /// <summary>
    /// Get Frame mode (bool)
    /// </summary>
    /// <returns>Bool</returns>
    bool getFrMod();
    /// <summary>
    /// Updates Instance matrix(transform)
    /// </summary>
    /// <param name="instId">valid instance id</param>
    /// <param name="newTransform">new transformation matrix</param>
    /// <returns>Nothing</returns>
    void updateInstanceTransform(uint32_t instId, glm::float4x4 newTransform);
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

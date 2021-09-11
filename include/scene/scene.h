#pragma once

#include "camera.h"
#include "glm-wrapper.hpp"
#include "materials.h"
#undef float4
#undef float3

#include <cstdint>
#include <set>
#include <stack>
#include <vector>


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
    glm::float3 massCenter;
};

class Scene
{
private:
    std::vector<Camera> mCameras;

    std::stack<uint32_t> mDelInstances;
    std::stack<uint32_t> mDelMesh;
    std::stack<uint32_t> mDelMaterial;

    bool FrMod{};

    std::set<uint32_t> mDirtyInstances;

public:
    struct Vertex
    {
        glm::float3 pos;
        uint32_t tangent;
        uint32_t normal;
        uint32_t uv;
    };

    // GPU side structure
    struct Light
    {
        glm::float4 points[4];
        glm::float4 color = glm::float4(1.0f);
    };

    // CPU side structure
    struct RectLightDesc
    {
        glm::float3 position; // world position
        glm::float3 orientation; // euler angles in degrees
        // OX - axis of light or normal
        float width; // OY
        float height; // OZ
        glm::float3 color;
    };

    std::vector<RectLightDesc> mLightDesc;

    enum class DebugView
    {
        eNone = 0,
        eNormals,
    };

    DebugView mDebugViewSettings = DebugView::eNone;

    bool transparentMode = true;
    bool opaqueMode = true;

    glm::float4 mLightPosition{ 10.0, 10.0, 10.0, 1.0 };

    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;
    std::vector<Light> mLights;

    std::vector<uint32_t> mTransparentInstances;
    std::vector<uint32_t> mOpaqueInstances;

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

    std::vector<Light>& getLights()
    {
        return mLights;
    }

    std::vector<RectLightDesc>& getLightsDesc()
    {
        return mLightDesc;
    }

    void addCamera(Camera camera)
    {
        mCameras.push_back(camera);
    }

    Camera& getCamera(uint32_t index)
    {
        assert(index < mCameras.size());
        return mCameras[index];
    }

    const std::vector<Camera>& getCameras() const
    {
        return mCameras;
    }

    size_t getCameraCount()
    {
        return mCameras.size();
    }

    const std::vector<Instance>& getInstances() const
    {
        return mInstances;
    }

    const std::vector<Mesh>& getMeshes() const
    {
        return mMeshes;
    }

    void updateCamerasParams(int width, int height)
    {
        for (Camera& camera : mCameras)
        {
            camera.updateAspectRatio((float)width / height);
        }
    }

    void removeLight(uint32_t lightId);
    void updateLight(uint32_t lightId, const RectLightDesc& desc);
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
    uint32_t createInstance(uint32_t meshId, uint32_t materialId, const glm::mat4& transform, const glm::float3& massCenter);

    uint32_t addMaterial(const Material& material);

    uint32_t createLight(const RectLightDesc& desc);
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

    std::vector<uint32_t>& getOpaqueInstancesToRender(const glm::float3& camPos);

    std::vector<uint32_t>& getTransparentInstancesToRender(const glm::float3& camPos);

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
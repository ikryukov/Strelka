#pragma once

#include "camera.h"
#include "glm-wrapper.hpp"
#include "materials.h"
#undef float4
#undef float3

#include <cstdint>
#include <set>
#include <stack>
#include <unordered_map>
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
    bool isLight = false;
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
        float pad0;
        float pad1;
    };

    struct Node
    {
        std::string name;
        glm::float3 translation;
        glm::float3 scale;
        glm::quat rotation;
        int parent = -1;
        std::vector<int> children;
    };
    std::vector<Node> mNodes;

    enum class AnimationState : uint32_t
    {
        eStop,
        ePlay,
        eScroll,
    };
    AnimationState mAnimState = AnimationState::eStop;
    struct AnimationSampler
    {
        enum class InterpolationType
        {
            LINEAR,
            STEP,
            CUBICSPLINE
        };
        InterpolationType interpolation;
        std::vector<float> inputs;
        std::vector<glm::float4> outputsVec4;
    };

    struct AnimationChannel
    {
        enum class PathType
        {
            TRANSLATION,
            ROTATION,
            SCALE
        };
        PathType path;
        int node;
        uint32_t samplerIndex;
    };

    struct Animation
    {
        std::string name;
        std::vector<AnimationSampler> samplers;
        std::vector<AnimationChannel> channels;
        float start = std::numeric_limits<float>::max();
        float end = std::numeric_limits<float>::min();
    };
    std::vector<Animation> mAnimations;

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
        float intensity;
    };

    std::vector<RectLightDesc> mLightDesc;

    enum class DebugView: uint32_t
    {
        eNone = 0,
        eNormals = 1,
        eShadows = 2,
        eLTC = 3,
        eMotion = 4,
        eCustomDebug = 5,
        ePTDebug = 11
    };

    DebugView mDebugViewSettings = DebugView::eNone;

    bool transparentMode = true;
    bool opaqueMode = true;

    glm::float4 mLightPosition{ 10.0, 10.0, 10.0, 1.0 };

    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::string modelPath;
    std::string getSceneFileName();
    std::string getSceneDir();

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;
    std::vector<Light> mLights;

    std::vector<uint32_t> mTransparentInstances;
    std::vector<uint32_t> mOpaqueInstances;

    Scene() = default;

    ~Scene() = default;

    std::unordered_map<uint32_t, uint32_t> mLightIdToInstanceId{};

    std::unordered_map<uint32_t, std::string> mMatIdToTexName{};
    std::unordered_map<int32_t, std::string> mTexIdToTexName{};

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
    void createLightMesh();


    glm::float4x4 getTransform(const Scene::RectLightDesc& desc)
    {
        const glm::float4x4 translationMatrix = glm::translate(glm::float4x4(1.0f), desc.position);
        glm::quat rotation = glm::quat(glm::radians(desc.orientation)); // to quaternion
        const glm::float4x4 rotationMatrix{ rotation };
        glm::float3 scale = { 1.0f, desc.width, desc.height };
        const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), scale);

        const glm::float4x4 localTransform = translationMatrix * rotationMatrix * scaleMatrix;

        return localTransform;
    }

    glm::float4x4 getTransformFromRoot(int nodeIdx)
    {
        std::stack<glm::float4x4> xforms;
        while (nodeIdx != -1)
        {
            const Node& n = mNodes[nodeIdx];
            glm::float4x4 xform = glm::translate(glm::float4x4(1.0f), n.translation) * glm::float4x4(n.rotation) * glm::scale(glm::float4x4(1.0f), n.scale);
            xforms.push(xform);
            nodeIdx = n.parent;
        }
        glm::float4x4 xform = glm::float4x4(1.0);
        while (!xforms.empty())
        {
            xform = xform * xforms.top();
            xforms.pop();
        }
        return xform;
    }

    glm::float4x4 getTransform(int nodeIdx)
    {
        glm::float4x4 xform = glm::float4x4(1.0);
        while (nodeIdx != -1)
        {
            const Node& n = mNodes[nodeIdx];
            xform = glm::translate(glm::float4x4(1.0f), n.translation) *
                    glm::float4x4(n.rotation) * glm::scale(glm::float4x4(1.0f), n.scale) * xform;
            nodeIdx = n.parent;
        }
        return xform;
    }


    glm::float4x4 getCameraTransform(int nodeIdx)
    {
        int child = mNodes[nodeIdx].children[0];
        return getTransform(child);
    }

    void updateAnimation(const float dt);

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

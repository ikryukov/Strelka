#pragma once

#include <vector>
#include <cstdint>
#include "glm-wrapper.hpp"
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

namespace nevk {

struct Vertex {
    glm::vec4 pos;
    glm::vec3 normal;
    glm::vec2 uv;     // hold the texture coordinate

    /*
     * A vertex should really only be considered equal
     * if both the position and the texture coordinate are the same.
     * This will cause our vertex class to only be unique
     * when one or more of its properties are different in comparison to another vertex.
     */
    bool operator==(const nevk::Vertex& other) const;
};

struct Mesh {
    uint32_t mIndex; // Index of 1st index in index buffer
    uint32_t mCount; // amount of indices in mesh
};


struct MeshInstance {
    /*
    * Defining an object that can hold a definition of which mesh and texture to use
    * and offer a way to update and fetch its transform matrix.
    * We need the transform matrix to know where and how the mesh should be positioned in the 3d world
    */
    const glm::mat4 identity;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotationAxis;
    float rotationDegrees;
    glm::mat4 transformMatrix;

    explicit MeshInstance(const glm::vec3& position = glm::vec3{ 0.0f, 0.0f, 0.0f },
        const glm::vec3& scale = glm::vec3{ 1.0f, 1.0f, 1.0f },
        const glm::vec3& rotationAxis = glm::vec3{ 0.0f, 1.0f, 0.0f },
        const float& rotationDegrees = 0.0f)
        :identity(glm::mat4{1.0f}),  // create the identity matrix needed for the subsequent matrix operations
         position(position),
         scale(scale),
         rotationAxis(rotationAxis),
         rotationDegrees(rotationDegrees),
         transformMatrix(identity) {};


    void update(const glm::mat4& projectionViewMatrix) {
        transformMatrix = projectionViewMatrix *
            glm::translate(identity, position) *
            glm::rotate(identity, glm::radians(rotationDegrees), rotationAxis) *
            glm::scale(identity, scale);
    }

    void rotateBy(const float& degrees) {  // функция поворота экземпляра
        rotationDegrees += degrees;

        if (rotationDegrees > 360.0f) {
            rotationDegrees -= 360.0f;
        }
        else if (rotationDegrees < -360.0f) {
            rotationDegrees += 360.0f;
        }
    }

    void init_update(const glm::mat4& projectionViewMatrix);

    void init_rotateBy(const float& degrees); // функция поворота экземпляра

    glm::mat4 getTransformMatrix() const;
};

//////////////////////////////////////////////////

struct Material {
    glm::vec4 color;
};

struct Instance {
    glm::mat4 transform;
    uint32_t mMeshId;
    uint32_t mMaterialId;
};

enum Mod {
    INSTANCE,
    MESH,
    MATERIAL,
};

class Scene {
private:
    //    Camera camera;
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    std::vector<Mesh> mMeshes;
    std::vector<Material> mMaterials;
    std::vector<Instance> mInstances;

public:
    Scene() = default;

    ~Scene() = default;

    // This is where the scene will perform any of its per frame logical operations
    // and is supplied with the delta for the current frame loop.
    // void update_scene(const float& delta) = 0;

    /// <summary>
    /// Transform matrix
    /// </summary>
    /// <returns>Nothing</returns>
    glm::mat4 createMeshTransform();
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
    void createMaterial(const glm::vec4& color);
    /// <summary>
    /// Adds instance/mesh/material
    /// </summary>
    /// <param name="meshId">valid mesh id</param>
    /// <param name="materialId">valid material id</param>
    /// <param name="transform">transform</param>
    /// <param name="vb">Vertices</param>
    /// <param name="ib">Indices</param>
    /// <param name="color">Color</param>
    /// <returns>Nothing</returns>
    void add(Mod mod,  uint32_t meshId, uint32_t materialId, glm::mat4& transform,
             std::vector<Vertex>& vb, std::vector<uint32_t>& ib,
             glm::vec4& color);    // instance, mesh, material    //MAKE THEM NULL !!!!!!!!!
    /// <summary>
    /// Removes instance/mesh/material
    /// </summary>
    /// <param name="meshId">valid mesh id</param>
    /// <param name="materialId">valid material id</param>
    /// <param name="instId">valid instance id</param>
    /// <returns>Nothing</returns>
    void remove(Mod mod, uint32_t meshId, uint32_t materialId, uint32_t instId); //MAKE THEM NULL !!!!!!!!!

    //    void update_camera();
};
} // namespace nevk

/*
 *  A template definition - a custom hashing function for a vertex.
 *  It takes the hash of the pos(position) and the hash of the uv fields, exclusive ors (XOR) them together
 *  while shifting the bits of the uv left.
 *  By combining the pos(position) and the uv into the hashing function,
 *  we can once again evaluate the uniqueness of a vertex correctly within our std::unordered_map.
 *  "FOR ASSETS" =>  std::unordered_map<ast::Vertex, uint32_t> uniqueVertices;
 */
namespace std {
template <>
struct hash<nevk::Vertex> {
    size_t operator()(const nevk::Vertex& vertex) const {
        return ((hash<glm::vec4>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.uv) << 1)) >> 1);
    }
};
} // namespace std
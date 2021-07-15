#include "modelloader.h"

#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>

namespace nevk
{

//  valid range of coordinates [-10; 10]
uint32_t packUV(const glm::float2& uv)
{
    int32_t packed = (uint32_t)((uv.x + 10.0f) / 20.0f * 16383.99999f);
    packed += (uint32_t)((uv.y + 10.0f) / 20.0f * 16383.99999f) << 16;
    return packed;
}

//  valid range of coordinates [-1; 1]
uint32_t packNormal(const glm::float3& normal)
{
    uint32_t packed = (uint32_t)((normal.x + 1.0f) / 2.0f * 511.99999f);
    packed += (uint32_t)((normal.y + 1.0f) / 2.0f * 511.99999f) << 10;
    packed += (uint32_t)((normal.z + 1.0f) / 2.0f * 511.99999f) << 20;
    return packed;
}

//  valid range of coordinates [-10; 10]
uint32_t packTangent(const glm::float3& tangent)
{
    uint32_t packed = (uint32_t)((tangent.x + 10.0f) / 20.0f * 511.99999f);
    packed += (uint32_t)((tangent.y + 10.0f) / 20.0f * 511.99999f) << 10;
    packed += (uint32_t)((tangent.z + 10.0f) / 20.0f * 511.99999f) << 20;
    return packed;
}

glm::float2 unpackUV(uint32_t val)
{
    glm::float2 uv;
    uv.y = ((val & 0xffff0000) >> 16) / 16383.99999f * 10.0f - 5.0f;
    uv.x = (val & 0x0000ffff) / 16383.99999f * 10.0f - 5.0f;

    return uv;
}

void ModelLoader::computeTangent(std::vector<Scene::Vertex>& vertices,
                                 const std::vector<uint32_t>& indices) const
{
    const size_t lastIndex = indices.size();
    Scene::Vertex& v0 = vertices[indices[lastIndex - 3]];
    Scene::Vertex& v1 = vertices[indices[lastIndex - 2]];
    Scene::Vertex& v2 = vertices[indices[lastIndex - 1]];

    glm::float2 uv0 = unpackUV(v0.uv);
    glm::float2 uv1 = unpackUV(v1.uv);
    glm::float2 uv2 = unpackUV(v2.uv);

    glm::float3 deltaPos1 = v1.pos - v0.pos;
    glm::float3 deltaPos2 = v2.pos - v0.pos;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    glm::vec3 tangent{ 0.0f, 0.0f, 1.0f };
    const float d = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
    if (abs(d) > 1e-6)
    {
        float r = 1.0f / d;
        tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    }

    glm::uint32_t packedTangent = packTangent(tangent);

    v0.tangent = packedTangent;
    v1.tangent = packedTangent;
    v2.tangent = packedTangent;
}

bool ModelLoader::loadModel(const std::string& modelFile, const std::string& mtlPath, nevk::Scene& scene)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelFile.c_str(), mtlPath.c_str(), true);
    if (!ret)
    {
        throw std::runtime_error(warn + err);
    }

    const bool hasMaterial = !mtlPath.empty() && !materials.empty();

    std::unordered_map<std::string, uint32_t> uniqueMaterial{};
    for (tinyobj::shape_t& shape : shapes)
    {
        uint32_t shapeMaterialId = 0; // TODO: make default material
        if (hasMaterial)
        {
            Scene::Material material{};
            const int materialIdx = shape.mesh.material_ids[0]; // assume that material per-shape
            const tinyobj::material_t& currMaterial = materials[materialIdx];
            const std::string& matName = currMaterial.name;

            if (uniqueMaterial.count(matName) == 0)
            {
                // need to create material
                material.ambient = { currMaterial.ambient[0],
                                     currMaterial.ambient[1],
                                     currMaterial.ambient[2], 1.0f };

                material.diffuse = { currMaterial.diffuse[0],
                                     currMaterial.diffuse[1],
                                     currMaterial.diffuse[2], 1.0f };

                material.specular = { currMaterial.specular[0],
                                      currMaterial.specular[1],
                                      currMaterial.specular[2], 1.0f };

                material.emissive = { currMaterial.emission[0],
                                      currMaterial.emission[1],
                                      currMaterial.emission[2], 1.0f };

                material.opticalDensity = currMaterial.ior;

                material.shininess = currMaterial.shininess;

                material.transparency = { currMaterial.transmittance[0],
                                          currMaterial.transmittance[1],
                                          currMaterial.transmittance[2], 1.0f };

                material.illum = currMaterial.illum;

                material.texAmbientId = mTexManager->loadTexture(currMaterial.ambient_texname, mtlPath);
                material.texDiffuseId = mTexManager->loadTexture(currMaterial.diffuse_texname, mtlPath);
                material.texSpecularId = mTexManager->loadTexture(currMaterial.specular_texname, mtlPath);
                material.texNormalId = mTexManager->loadTexture(currMaterial.bump_texname, mtlPath);
                material.d = currMaterial.dissolve;

                uint32_t matId = scene.createMaterial(material.ambient, material.diffuse,
                                                      material.specular, material.emissive,
                                                      material.transparency, material.opticalDensity,
                                                      material.shininess, material.illum,
                                                      material.texAmbientId, material.texDiffuseId,
                                                      material.texSpecularId, material.texNormalId,
                                                      material.d);

                uniqueMaterial[matName] = matId;
                shapeMaterialId = matId;
            }
            else
            {
                // reuse existing material
                shapeMaterialId = uniqueMaterial[matName];
            }
        }

        std::vector<Scene::Vertex> _vertices;
        std::vector<uint32_t> _indices;
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            tinyobj::index_t& idx0 = shape.mesh.indices[f + 0];
            tinyobj::index_t& idx1 = shape.mesh.indices[f + 1];
            tinyobj::index_t& idx2 = shape.mesh.indices[f + 2];

            const int verticesPerFace = shape.mesh.num_face_vertices[f];
            assert(verticesPerFace == 3); // ensure that we load triangulated faces
            for (int v = 0; v < verticesPerFace; ++v)
            {
                Scene::Vertex vertex{};
                const auto& idx = shape.mesh.indices[index_offset + v];
                vertex.pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                if (attrib.texcoords.empty())
                {
                    vertex.uv = packUV(glm::float2(0.0f, 0.0f));
                }
                else
                {
                    if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                        (idx2.texcoord_index < 0))
                    {
                        vertex.uv = packUV(glm::float2(0.0f, 0.0f));
                    }
                    else
                    {
                        vertex.uv = packUV(glm::float2(attrib.texcoords[2 * idx.texcoord_index + 0],
                                                       1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]));
                    }
                }

                if (attrib.normals.empty())
                {
                    vertex.normal = packNormal(glm::float3(0.0f, 0.0f, 0.0f));
                }
                else
                {
                    vertex.normal = packNormal(glm::float3(attrib.normals[3 * idx.normal_index + 0],
                                                           attrib.normals[3 * idx.normal_index + 1],
                                                           attrib.normals[3 * idx.normal_index + 2]));
                }

                _indices.push_back(static_cast<uint32_t>(_vertices.size()));
                _vertices.push_back(vertex);
            }
            index_offset += verticesPerFace;

            computeTangent(_vertices, _indices);
        }

        glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
        for (const Scene::Vertex& vertPos : _vertices)
        {
            sum += vertPos.pos;
        }
        glm::float3 massCenter = glm::float3(sum.x / _vertices.size(),
                                             sum.y / _vertices.size(),
                                             sum.z / _vertices.size());


        uint32_t meshId = scene.createMesh(_vertices, _indices);
        assert(meshId != -1);
        glm::float4x4 transform{ 1.0f };
        glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
        uint32_t instId = scene.createInstance(meshId, shapeMaterialId, transform, massCenter);
        assert(instId != -1);
    }

    //mTexManager->createTextureSampler();

    return ret;
}

void processPrimitive(const tinygltf::Model& model, nevk::Scene& scene, const tinygltf::Primitive& primitive, const glm::float4x4& transform, const float globalScale)
{
    using namespace std;
    assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

    const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
    const tinygltf::BufferView& positionView = model.bufferViews[positionAccessor.bufferView];
    const float* positionData = reinterpret_cast<const float*>(&model.buffers[positionView.buffer].data[positionAccessor.byteOffset + positionView.byteOffset]);
    assert(positionData != nullptr);
    const uint32_t vertexCount = static_cast<uint32_t>(positionAccessor.count);
    assert(vertexCount != 0);
    const int byteStride = positionAccessor.ByteStride(positionView);
    assert(byteStride > 0); // -1 means invalid glTF
    int posStride = byteStride / sizeof(float);

    // Normals
    const float* normalsData = nullptr;
    int normalStride = 0;
    if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
    {
        const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& normView = model.bufferViews[normalAccessor.bufferView];
        normalsData = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normalAccessor.byteOffset + normView.byteOffset]));
        assert(normalsData != nullptr);
        normalStride = normalAccessor.ByteStride(normView) / sizeof(float);
        assert(normalStride > 0);
    }

    // UVs
    const float* texCoord0Data = nullptr;
    int texCoord0Stride = 0;
    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
    {
        const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
        texCoord0Data = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
        texCoord0Stride = uvAccessor.ByteStride(uvView) / sizeof(float);
    }

    int matId = primitive.material;
    if (matId == -1)
    {
        matId = 0; // TODO: should be index of default material
    }

    glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
    std::vector<nevk::Scene::Vertex> vertices;
    vertices.reserve(vertexCount);
    for (uint32_t v = 0; v < vertexCount; ++v)
    {
        nevk::Scene::Vertex vertex{};
        vertex.pos = glm::make_vec3(&positionData[v * posStride]) * globalScale;
        vertex.normal = packNormal(glm::normalize(glm::vec3(normalsData ? glm::make_vec3(&normalsData[v * normalStride]) : glm::vec3(0.0f))));
        vertex.uv = packUV(texCoord0Data ? glm::make_vec2(&texCoord0Data[v * texCoord0Stride]) : glm::vec3(0.0f));
        vertices.push_back(vertex);
        sum += vertex.pos;
    }
    const glm::float3 massCenter = sum / (float)vertexCount;

    uint32_t indexCount = 0;
    std::vector<uint32_t> indices;
    const bool hasIndices = (primitive.indices != -1);
    assert(hasIndices); // currently support only this mode
    if (hasIndices)
    {
        const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        indexCount = static_cast<uint32_t>(accessor.count);
        assert(indexCount != 0 && (indexCount % 3 == 0));
        const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

        indices.reserve(indexCount);

        switch (accessor.componentType)
        {
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
            const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
            for (size_t index = 0; index < indexCount; index++)
            {
                indices.push_back(buf[index]);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
            const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
            for (size_t index = 0; index < indexCount; index++)
            {
                indices.push_back(buf[index]);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
            const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
            for (size_t index = 0; index < indexCount; index++)
            {
                indices.push_back(buf[index]);
            }
            break;
        }
        default:
            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
            return;
        }
    }

    uint32_t meshId = scene.createMesh(vertices, indices);
    assert(meshId != -1);
    uint32_t instId = scene.createInstance(meshId, matId, transform, massCenter);
    assert(instId != -1);
}

void processMesh(const tinygltf::Model& model, nevk::Scene& scene, const tinygltf::Mesh& mesh, const glm::float4x4& transform, const float globalScale)
{
    using namespace std;
    cout << "Mesh name: " << mesh.name << endl;
    cout << "Primitive count: " << mesh.primitives.size() << endl;
    for (size_t i = 0; i < mesh.primitives.size(); ++i)
    {
        processPrimitive(model, scene, mesh.primitives[i], transform, globalScale);
    }
}

glm::float4x4 getTransform(const tinygltf::Node& node, const float globalScale)
{
    if (node.matrix.empty())
    {
        glm::float3 scale{ 1.0f };
        if (!node.scale.empty())
        {
            scale = glm::float3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);
            // check that scale is uniform, otherwise we have to support it in shader
            // assert(scale.x == scale.y && scale.y == scale.z);
        }

        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (!node.rotation.empty())
        {
            const float floatRotation[4] = {
                (float)node.rotation[0],
                (float)node.rotation[1],
                (float)node.rotation[2],
                (float)node.rotation[3]
            };
            rotation = glm::make_quat(floatRotation);
        }

        glm::float3 translation{ 0.0f };
        if (!node.translation.empty())
        {
            translation = glm::float3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]);
            translation *= globalScale;
        }

        const glm::float4x4 translationMatrix = glm::translate(glm::float4x4(1.0f), translation);
        const glm::float4x4 rotationMatrix{ rotation };
        const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), scale);

        const glm::float4x4 localTransform = translationMatrix * rotationMatrix * scaleMatrix;

        return localTransform;
    }
    else
    {
        glm::float4x4 localTransform = glm::make_mat4(node.matrix.data());
        return localTransform;
    }
}

void processNode(const tinygltf::Model& model, nevk::Scene& scene, const tinygltf::Node& node, const glm::float4x4& baseTransform, const float globalScale)
{
    using namespace std;
    cout << "Node name: " << node.name << endl;

    const glm::float4x4 localTransform = getTransform(node, globalScale);
    const glm::float4x4 globalTransform = baseTransform * localTransform;

    if (node.mesh != -1) // mesh exist
    {
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        processMesh(model, scene, mesh, globalTransform, globalScale);
    }
    else if (node.camera != -1) // camera node
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(globalTransform, scale, rotation, translation, skew, perspective);
        scene.getCamera(node.camera).position = translation;
        scene.getCamera(node.camera).mOrientation = rotation;
        scene.getCamera(node.camera).updateViewMatrix();
    }

    for (int i = 0; i < node.children.size(); ++i)
    {
        processNode(model, scene, model.nodes[node.children[i]], globalTransform, globalScale);
    }
}

VkSamplerAddressMode getVkWrapMode(int32_t wrapMode)
{
    switch (wrapMode)
    {
    case 10497:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case 33071:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case 33648:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    default:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkFilter getVkFilterMode(int32_t filterMode)
{
    switch (filterMode)
    {
    case 9728:
        return VK_FILTER_NEAREST;
    case 9729:
        return VK_FILTER_LINEAR;
    case 9984:
        return VK_FILTER_NEAREST;
    case 9985:
        return VK_FILTER_NEAREST;
    case 9986:
        return VK_FILTER_LINEAR;
    case 9987:
        return VK_FILTER_LINEAR;
    default:
        return VK_FILTER_LINEAR;
    }
}
std::unordered_map<uint32_t, uint32_t> texIdToModelSamp{};
std::unordered_map<uint32_t, uint32_t> modelSampIdToLoadedSampId{};

void findTextureSamplers(const tinygltf::Model& model, nevk::Scene& scene, nevk::TextureManager& textureManager)
{
    nevk::TextureManager::TextureSamplerDesc currentSamplerDesc{};
    uint32_t samplerNumber = 0;

    for (const tinygltf::Sampler& sampler : model.samplers)
    {
        currentSamplerDesc = { getVkFilterMode(sampler.minFilter), getVkFilterMode(sampler.magFilter), getVkWrapMode(sampler.wrapS), getVkWrapMode(sampler.wrapT) };
        modelSampIdToLoadedSampId[samplerNumber] = textureManager.sampDescToId.find(currentSamplerDesc)->second;
        ++samplerNumber;
    }
}

void loadTextures(const tinygltf::Model& model, nevk::Scene& scene, nevk::TextureManager& textureManager)
{
    texIdToModelSamp[-1] = 0;
    for (const tinygltf::Texture& tex : model.textures)
    {
        const tinygltf::Image& image = model.images[tex.source];
        // TODO: create sampler for tex

        if (image.component == 3)
        {
            // unsupported
            return;
        }
        else if (image.component == 4)
        {
            // supported
        }
        else
        {
            // error
        }

        const void* data = image.image.data();
        uint32_t width = image.width;
        uint32_t height = image.height;

        const std::string name = image.uri;

        int texId = textureManager.loadTextureGltf(data, width, height, name);
        assert(texId != -1);

        texIdToModelSamp[texId] = modelSampIdToLoadedSampId.find(tex.sampler)->second;
    }
}

void loadMaterials(const tinygltf::Model& model, nevk::Scene& scene, nevk::TextureManager& textureManager)
{
    for (const tinygltf::Material& material : model.materials)
    {
        Scene::Material currMaterial{};

        currMaterial.diffuse = glm::float4(material.pbrMetallicRoughness.baseColorFactor[0],
                                           material.pbrMetallicRoughness.baseColorFactor[1],
                                           material.pbrMetallicRoughness.baseColorFactor[2],
                                           material.pbrMetallicRoughness.baseColorFactor[3]);
        currMaterial.texNormalId = material.normalTexture.index;
        currMaterial.sampNormalId = texIdToModelSamp.find(currMaterial.texNormalId)->second;

        currMaterial.baseColorFactor = glm::float4(material.pbrMetallicRoughness.baseColorFactor[0],
                                                   material.pbrMetallicRoughness.baseColorFactor[1],
                                                   material.pbrMetallicRoughness.baseColorFactor[2],
                                                   material.pbrMetallicRoughness.baseColorFactor[3]);

        currMaterial.texBaseColor = material.pbrMetallicRoughness.baseColorTexture.index;
        currMaterial.sampBaseId = texIdToModelSamp.find(currMaterial.texBaseColor)->second;

        currMaterial.roughnessFactor = (float)material.pbrMetallicRoughness.roughnessFactor;
        currMaterial.metallicFactor = (float)material.pbrMetallicRoughness.metallicFactor;

        currMaterial.emissiveFactor = glm::float3(material.emissiveFactor[0],
                                                  material.emissiveFactor[1],
                                                  material.emissiveFactor[2]);
        currMaterial.texEmissive = material.emissiveTexture.index;
       currMaterial.sampEmissiveId = texIdToModelSamp.find(currMaterial.texEmissive)->second;

        currMaterial.texOcclusion = material.occlusionTexture.index;
        currMaterial.sampOcclusionId = texIdToModelSamp.find(currMaterial.texOcclusion)->second;

        currMaterial.d = (float)material.pbrMetallicRoughness.baseColorFactor[3];

        currMaterial.illum = material.alphaMode == "OPAQUE" ? 2 : 1;
        scene.addMaterial(currMaterial);
    }
}

void loadCameras(const tinygltf::Model& model, nevk::Scene& scene)
{
    for (uint32_t i = 0; i < model.cameras.size(); ++i)
    {
        const tinygltf::Camera& cameraGltf = model.cameras[i];
        if (strcmp(cameraGltf.type.c_str(), "perspective") == 0)
        {
            nevk::Camera camera;
            camera.fov = cameraGltf.perspective.yfov * (180.0f / 3.1415926f);
            camera.znear = cameraGltf.perspective.znear;
            camera.zfar = cameraGltf.perspective.zfar;
            camera.name = cameraGltf.name;
            scene.addCamera(camera);
        }
        else
        {
            // not supported
        }
    }
    if (scene.getCameraCount() == 0)
    {
        // add default camera
        Camera camera;
        camera.updateViewMatrix();
        scene.addCamera(camera);
    }
}

bool ModelLoader::loadModelGltf(const std::string& modelPath, nevk::Scene& scene)
{
    if (modelPath.empty())
    {
        return false;
    }

    using namespace std;
    tinygltf::Model model;
    tinygltf::TinyGLTF gltf_ctx;
    std::string err;
    std::string warn;
    bool res = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, modelPath.c_str());
    if (!res)
    {
        cerr << "Unable to load file: " << modelPath << endl;
        return res;
    }
    for (int i = 0; i < model.scenes.size(); ++i)
    {
        cout << "Scene: " << model.scenes[i].name << endl;
    }

    int sceneId = model.defaultScene;

    findTextureSamplers(model, scene, *mTexManager);
    loadTextures(model, scene, *mTexManager);
    loadMaterials(model, scene, *mTexManager);

    loadCameras(model, scene);

    const float globalScale = 1.0f;

    for (int i = 0; i < model.scenes[sceneId].nodes.size(); ++i)
    {
        const int rootNodeIdx = model.scenes[sceneId].nodes[i];
        processNode(model, scene, model.nodes[rootNodeIdx], glm::float4x4(1.0f), globalScale);
    }
    return res;
}
} // namespace nevk

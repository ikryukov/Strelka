#include "modelloader.h"

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

void Model::computeTangent(std::vector<Scene::Vertex>& vertices,
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

bool Model::loadModel(const std::string& modelFile, const std::string& mtlPath, nevk::Scene& mScene)
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
    bool transparent = false;
    for (tinyobj::shape_t& shape : shapes)
    {
        uint32_t shapeMaterialId = 0; // TODO: make default material
        if (hasMaterial)
        {
            Scene::Material material{};
            const int materialIdx = shape.mesh.material_ids[0]; // assume that material per-shape
            const tinyobj::material_t& currMaterial = materials[materialIdx];
            const std::string& matName = currMaterial.name;
            const std::string& bumpTexname = currMaterial.bump_texname;

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

                uint32_t matId = mScene.createMaterial(material.ambient, material.diffuse,
                                                       material.specular, material.emissive,
                                                       material.transparency, material.opticalDensity,
                                                       material.shininess, material.illum,
                                                       material.texAmbientId, material.texDiffuseId,
                                                       material.texSpecularId, material.texNormalId);

                bool tr_illum = std::find(_transparent_illums.begin(),
                                          _transparent_illums.end(), material.illum) != _transparent_illums.end();
                if (tr_illum)
                {
                    transparent = true;
                }
                else
                {
                    transparent = false;
                }

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
            for (size_t v = 0; v < verticesPerFace; ++v)
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
                vertex.materialId = shapeMaterialId;

                _indices.push_back(static_cast<uint32_t>(_vertices.size()));
                _vertices.push_back(vertex);
            }
            index_offset += verticesPerFace;

            computeTangent(_vertices, _indices);
        }

        uint32_t meshId = mScene.createMesh(_vertices, _indices);
        glm::float4x4 transform{ 1.0f };
        glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
        uint32_t instId = mScene.createInstance(meshId, shapeMaterialId, transform);

        glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
        for (Scene::Vertex& vertPos : _vertices)
        {
            sum += vertPos.pos;
        }
        glm::float3 objCenter = glm::float3(sum.x / _vertices.size(),
                                            sum.y / _vertices.size(),
                                            sum.z / _vertices.size());

        if (transparent)
        {
            mScene.mTransparentInstances.push_back(instId);
            mScene.massCenterTr[instId] = objCenter;
        }
        else
        {
            mScene.mOpaqueInstances.push_back(instId);
            mScene.massCenterOp[instId] = objCenter;
        }
    }

    mTexManager->createTextureSampler();

    return ret;
}
} // namespace nevk

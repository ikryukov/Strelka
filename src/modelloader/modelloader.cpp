#include "modelloader.h"

namespace nevk
{

//  valid range of coordinates [-6; 6]
uint32_t packUV(const glm::float2& uv)
{
    int32_t packed = (uint32_t)((uv.x + 6.0f) / 12.0f * 16383.99999f);
    packed += (uint32_t)((uv.y + 6.0f) / 12.0f * 16383.99999f) << 16;
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

glm::float2 unpackUV(uint32_t val)
{
    glm::float2 uv;
    uv.y = ((val & 0xffff0000) >> 16) / 16383.99999f * 10.0f - 5.0f;
    uv.x = (val & 0x0000ffff) / 16383.99999f * 10.0f - 5.0f;

    return uv;
}

void Model::computeTangent(size_t index_offset)
{
    Scene::Vertex& v0 = _vertices[_indices[index_offset - 3]];
    Scene::Vertex& v1 = _vertices[_indices[index_offset - 2]];
    Scene::Vertex& v2 = _vertices[_indices[index_offset - 1]];

    glm::float2 uv0 = unpackUV(v0.uv);
    glm::float2 uv1 = unpackUV(v1.uv);
    glm::float2 uv2 = unpackUV(v2.uv);

    glm::float3 deltaPos1 = v1.pos - v0.pos;
    glm::float3 deltaPos2 = v2.pos - v0.pos;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

    v0.tangent = tangent;
    v1.tangent = tangent;
    v2.tangent = tangent;
}

bool Model::loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), MTL_PATH.c_str(), true);
    if (!ret)
    {
        throw std::runtime_error(warn + err);
    }
    std::unordered_map<std::string, uint32_t> unMat{};
    for (auto& shape : shapes)
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            tinyobj::index_t idx0 = shape.mesh.indices[f + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[f + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[f + 2];
            bool isBumpTex = false;

            int fv = shape.mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++)
            {
                Scene::Vertex vertex{};
                auto idx = shape.mesh.indices[index_offset + v];
                vertex.pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                if (attrib.texcoords.empty())
                {
                    vertex.uv = packUV({ 0.0f, 0.0f });
                }
                else
                {
                    if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                        (idx2.texcoord_index < 0))
                    {
                        vertex.uv = packUV({ 0.0f, 0.0f });
                    }
                    else
                        vertex.uv = packUV({ attrib.texcoords[2 * idx.texcoord_index + 0],
                                             1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] });
                }


                if (attrib.normals.empty())
                {
                    vertex.normal = packNormal({ 0.0f, 0.0f, 0.0f });
                }
                else
                {
                    vertex.normal = packNormal({ attrib.normals[3 * idx.normal_index + 0],
                                                 attrib.normals[3 * idx.normal_index + 1],
                                                 attrib.normals[3 * idx.normal_index + 2] });
                }

                Scene::Material material{};
                if (!MTL_PATH.empty())
                {
                    std::string matName = materials[shape.mesh.material_ids[f]].name;
                    std::string bumpTexname = materials[shape.mesh.material_ids[f]].bump_texname;
                    if (!bumpTexname.empty())
                    {
                        isBumpTex = true;
                    }
                    if (unMat.count(matName) == 0)
                    {
                        material.ambient = { materials[shape.mesh.material_ids[f]].ambient[0],
                                             materials[shape.mesh.material_ids[f]].ambient[1],
                                             materials[shape.mesh.material_ids[f]].ambient[2], 1.0f };

                        material.diffuse = { materials[shape.mesh.material_ids[f]].diffuse[0],
                                             materials[shape.mesh.material_ids[f]].diffuse[1],
                                             materials[shape.mesh.material_ids[f]].diffuse[2], 1.0f };

                        material.specular = { materials[shape.mesh.material_ids[f]].specular[0],
                                              materials[shape.mesh.material_ids[f]].specular[1],
                                              materials[shape.mesh.material_ids[f]].specular[2], 1.0f };

                        material.emissive = { materials[shape.mesh.material_ids[f]].emission[0],
                                              materials[shape.mesh.material_ids[f]].emission[1],
                                              materials[shape.mesh.material_ids[f]].emission[2], 1.0f };

                        material.opticalDensity = materials[shape.mesh.material_ids[f]].ior;

                        material.shininess = materials[shape.mesh.material_ids[f]].shininess;

                        material.transparency = { materials[shape.mesh.material_ids[f]].transmittance[0],
                                                  materials[shape.mesh.material_ids[f]].transmittance[1],
                                                  materials[shape.mesh.material_ids[f]].transmittance[2], 1.0f };

                        material.illum = materials[shape.mesh.material_ids[f]].illum;

                        material.texAmbientId = mTexManager->loadTexture(materials[shape.mesh.material_ids[f]].ambient_texname, MTL_PATH);

                        material.texDiffuseId = mTexManager->loadTexture(materials[shape.mesh.material_ids[f]].diffuse_texname, MTL_PATH);

                        material.texSpecularId = mTexManager->loadTexture(materials[shape.mesh.material_ids[f]].specular_texname, MTL_PATH);

                        material.texNormalId = mTexManager->loadTexture(materials[shape.mesh.material_ids[f]].bump_texname, MTL_PATH);

                        uint32_t matId = mScene.createMaterial(material.ambient, material.diffuse,
                                                               material.specular, material.emissive,
                                                               material.opticalDensity, material.shininess,
                                                               material.transparency, material.illum,
                                                               material.texAmbientId, material.texDiffuseId,
                                                               material.texSpecularId, material.texNormalId);
                        unMat[matName] = matId;
                        vertex.materialId = matId;
                    }
                    else
                    {
                        vertex.materialId = unMat[matName];
                    }
                }

                _indices.push_back(static_cast<uint32_t>(_vertices.size()));
                _vertices.push_back(vertex);
            }
            index_offset += fv;

            if (isBumpTex)
            {
                computeTangent(index_offset);
            }
        }
    }

    mTexManager->createTextureSampler();
    uint32_t meshId = mScene.createMesh(_vertices, _indices);
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = mScene.createInstance(meshId, -1, transform);
    return ret;
}
} // namespace nevk

#include "modelloader.h"

namespace nevk
{
glm::float1 packUV(const glm::float2& uv)
{
    unsigned int packed = (unsigned int)((uv.x + 1.0f) * 127.99999f);
    packed += (unsigned int)((uv.y) * 127.99999f) << 8;

    return *((glm::float1*)(&packed));
}

glm::float1 packNormal(const glm::float3& normal)
{
    unsigned int packed = (unsigned int)((normal.x + 1.0f) * 127.99999f);
    packed += (unsigned int)((normal.y) * 127.99999f) << 8;
    packed += (unsigned int)((normal.z + 1.0f) * 127.99999f) << 16;

    return *((glm::float1*)(&packed));
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

                glm::float2 uv = {
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                };

                vertex.uv = packUV(uv);

                glm::float3 normal = {attrib.normals[3 * idx.normal_index + 0],
                                      attrib.normals[3 * idx.normal_index + 1],
                                      attrib.normals[3 * idx.normal_index + 2]};

                vertex.normal = packNormal(normal);

                Scene::Material material{};
                if (!MTL_PATH.empty())
                {
                    std::string matName = materials[shape.mesh.material_ids[f]].name;
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
        }
    }

    uint32_t meshId = mScene.createMesh(_vertices, _indices);
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = mScene.createInstance(meshId, -1, transform);

    return ret;
}
} // namespace nevk

#include "modelloader.h"

namespace nevk
{
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

    for (auto& shape : shapes)
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            tinyobj::index_t idx0 = shape.mesh.indices[f + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[f + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[f + 2];


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
                    vertex.uv = {
                        0.0f, 0.0f
                    };
                }
                else
                {
                    if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                        (idx2.texcoord_index < 0))
                    {
                        vertex.uv = {
                            0.0f, 0.0f
                        };
                    }
                    else
                        vertex.uv = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                        };
                }


                if (attrib.normals.empty())
                {
                    vertex.normal = {
                        0.0f, 0.0f, 0.0f
                    };
                }
                else
                {
                    vertex.normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                }

                Scene::Material material{};
                if (!MTL_PATH.empty())
                {

                    material.ambient = { materials[shape.mesh.material_ids[f]].ambient[0],
                                         materials[shape.mesh.material_ids[f]].ambient[1],
                                         materials[shape.mesh.material_ids[f]].ambient[2] };

                    material.diffuse = { materials[shape.mesh.material_ids[f]].diffuse[0],
                                         materials[shape.mesh.material_ids[f]].diffuse[1],
                                         materials[shape.mesh.material_ids[f]].diffuse[2] };

                    material.specular = { materials[shape.mesh.material_ids[f]].specular[0],
                                          materials[shape.mesh.material_ids[f]].specular[1],
                                          materials[shape.mesh.material_ids[f]].specular[2] };

                    material.emissive = { materials[shape.mesh.material_ids[f]].emission[0],
                                          materials[shape.mesh.material_ids[f]].emission[1],
                                          materials[shape.mesh.material_ids[f]].emission[2] };

                    material.opticalDensity = materials[shape.mesh.material_ids[f]].ior;

                    material.shininess = materials[shape.mesh.material_ids[f]].shininess;

                    material.transparency = { materials[shape.mesh.material_ids[f]].transmittance[0],
                                              materials[shape.mesh.material_ids[f]].transmittance[1],
                                              materials[shape.mesh.material_ids[f]].transmittance[2] };

                    material.illum = materials[shape.mesh.material_ids[f]].illum;

                    material.texAmbientId = 0;

                    material.texDiffuseId = 0;

                    material.texSpeculaId = 0;

                    material.texNormalId = 0;
                }


                uint32_t matId = mScene.createMaterial(material.ambient, material.diffuse,
                                                       material.specular, material.emissive,
                                                       material.opticalDensity, material.shininess,
                                                       material.transparency, material.illum,
                                                       material.texAmbientId, material.texDiffuseId,
                                                       material.texSpeculaId, material.texNormalId);

                std::string matName = materials[shape.mesh.material_ids[f]].name;
                std::unordered_map<std::string, uint32_t> unMat{};

                if (unMat.count(matName) == 0)
                {
                    unMat[matName] = vertex.materialId;
                    vertex.materialId = matId;
                }
                else
                {
                    vertex.materialId = unMat[matName];
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

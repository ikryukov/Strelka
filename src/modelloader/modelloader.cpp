#include "modelloader.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

namespace nevk
{

//  valid range of coordinates [-5; 5]
uint32_t packUV(const glm::float2& uv)
{
    int32_t packed = (uint32_t)((uv.x + 5.0f) / 10.0f * 16383.99999f);
    packed += (uint32_t)((uv.y + 5.0f) / 10.0f * 16383.99999f) << 16;
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

bool Model::loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene, glm::float3 camPosition)
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
    bool transparent = false;
    int count = 0;
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
                                                               material.transparency, material.opticalDensity,
                                                               material.shininess,
                                                               material.illum,
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

                _inds.push_back(static_cast<uint32_t>(_vertices.size()));
                _verts.push_back(vertex);
            }
            index_offset += fv;
        }
        if (transparent)
        {
            std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>> tmp;
            tmp[_inds] = _verts;
            _transparent_objects.push_back(tmp);
            std::vector<Scene::Vertex> v;
            _verts = v;
        }
        else
        {
            std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>> tmp;
            tmp[_inds] = _verts;
            _opaque_objects.push_back(tmp);
            std::vector<Scene::Vertex> v;
            _verts = v;
        }
    }

    mTexManager->createTextureSampler();
    sortMaterials(camPosition);
    uint32_t meshId = mScene.createMesh(_vertices, _indices);
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = mScene.createInstance(meshId, -1, transform);
    return ret;
}

void Model::parseVertsInds(std::vector<std::map<std::vector<uint32_t>,
                                          std::vector<Scene::Vertex>>> vec){
    _vertices.clear();
    for (auto& el:vec){
        for (auto& item:el){
            for (Scene::Vertex vert:item.second){
                _vertices.push_back(vert);
            }
        }
    }
}

bool comp (std::map<std::vector<uint32_t>, glm::float3> a,
          std::map<std::vector<uint32_t>, glm::float3> b) {
    std::map<std::vector<uint32_t>, glm::float3>::iterator it1 = a.begin();
    std::map<std::vector<uint32_t>, glm::float3>::iterator it2 = b.begin();
    return (it1->second).x < (it2->second).x &&
           (it1->second).y < (it2->second).y &&
           (it1->second).z < (it2->second).z;
}

void Model::sortMaterials(glm::float3 camPosition)
{
    std::vector<std::map<std::vector<uint32_t>, glm::float3>> dist;
    for (auto& obj:_transparent_objects){
        std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>>::iterator it = obj.begin();
        std::vector<uint32_t> indices = it->first;
        std::vector<Scene::Vertex> objVerts = it->second;
        glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);;
        for (auto vertPos:objVerts){
            sum += vertPos.pos;
        }
        glm::float3 objCenter = glm::float3(sum.x / objVerts.size(),
                                         sum.y / objVerts.size(),
                                         sum.z / objVerts.size()) ;

        std::map<std::vector<uint32_t>, glm::float3> tmp;
        tmp[indices] = (camPosition - objCenter);
        dist.push_back(tmp);
    }

    if (dist.size() > 1)
    {
        std::sort(dist.begin(), dist.end(), comp);

        for (auto& el1:dist){
            for (auto& el2:_transparent_objects){
                std::map<std::vector<uint32_t>, glm::float3>::iterator itEl1 = el1.begin();
                std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>>::iterator itEl2 = el2.begin();
                std::vector<uint32_t> indices1 = itEl1->first;
                std::vector<uint32_t> indices2 = itEl2->first;
                if (indices1 == indices2){
                    _opaque_objects.push_back(el2);
                    parseVertsInds(_opaque_objects);
                }
            }
        }
    }
}
} // namespace nevk

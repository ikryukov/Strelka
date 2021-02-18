#include "modelloader.h"

namespace nevk
{
bool Model::loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), MTL_PATH.c_str(), false);
    if (!ret)
    {
        throw std::runtime_error(warn + err);
    }
//    std::unordered_map<Scene::Vertex, uint32_t> uniqueVertices; //
    for (size_t s = 0; s < shapes.size(); s++)
    {
        int i = 0;
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++)
            {
                Scene::Vertex vertex{};
                auto idx = shapes[s].mesh.indices[index_offset + v];
                vertex.pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                vertex.uv = {
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                };

                vertex.normal = {
                    attrib.normals[3 * idx.normal_index + 0],
                    attrib.normals[3 * idx.normal_index + 1],
                    attrib.normals[3 * idx.normal_index + 2]
                };

//                /////
//                vertex.tangent = {};
//                vertex.bitangent = {};
//
//                if (uniqueVertices.count(vertex) == 0) {
//                    uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
//                    _vertices.push_back(vertex);
//                }
//
//                _indices.push_back(uniqueVertices[vertex]);
//
//                i++;
//                if (!(i % 3)) {
//                    size_t indicesSize = _indices.size();
//                    uint32_t i0 = _indices[indicesSize - 3];
//                    uint32_t i1 = _indices[indicesSize - 2];
//                    uint32_t i2 = _indices[indicesSize - 1];
//                    Scene::Vertex &v0 = _vertices[i0];
//                    Scene::Vertex &v1 = _vertices[i1];
//                    Scene::Vertex &v2 = _vertices[i2];
//
//                    glm::vec2 duv1 = v1.uv - v0.uv;
//                    glm::vec2 duv2 = v2.uv - v0.uv;
//                    float k = 1 / (duv1.x*duv2.y - duv2.x*duv1.y);
//                    glm::mat2x2 UV(duv2.y, -duv1.y, -duv2.x, duv1.x);
//                    glm::mat2x3 E(v1.pos - v0.pos, v2.pos - v0.pos);
//                    glm::mat2x3 TB = k*E*UV;
//
//                    v0.tangent += TB[0];
//                    v0.bitangent += TB[1];
//                    v1.tangent += TB[0];
//                    v1.bitangent += TB[1];
//                    v2.tangent += TB[0];
//                    v2.bitangent += TB[1];
//                }
//                /////

                vertex.color = { attrib.colors[3 * idx.vertex_index + 0],
                                 attrib.colors[3 * idx.vertex_index + 1],
                                 attrib.colors[3 * idx.vertex_index + 2] };

                if (!MTL_PATH.empty())
                {
                    vertex.ka = { materials[shapes[s].mesh.material_ids[f]].ambient[0],
                                  materials[shapes[s].mesh.material_ids[f]].ambient[1],
                                  materials[shapes[s].mesh.material_ids[f]].ambient[2] };

                    vertex.kd = { materials[shapes[s].mesh.material_ids[f]].diffuse[0],
                                  materials[shapes[s].mesh.material_ids[f]].diffuse[1],
                                  materials[shapes[s].mesh.material_ids[f]].diffuse[2] };

                    vertex.ks = { materials[shapes[s].mesh.material_ids[f]].specular[0],
                                  materials[shapes[s].mesh.material_ids[f]].specular[1],
                                  materials[shapes[s].mesh.material_ids[f]].specular[2] };
                }

                _indices.push_back(static_cast<uint32_t>(_vertices.size()));
                _vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }
//    /////
//    for (int i = 0; i < _vertices.size(); i++) {
//        _vertices[i].tangent = glm::normalize(_vertices[i].tangent);
//        _vertices[i].bitangent = glm::normalize(_vertices[i].bitangent);
//        glm::vec3 &T = _vertices[i].tangent;
//        glm::vec3 &B = _vertices[i].bitangent;
//        glm::vec3 &N = _vertices[i].normal;
//        T = glm::normalize(T - glm::dot(T, N) * N);
//        B = glm::cross(N, T);
//    }
//    /////

    uint32_t meshId = mScene.createMesh(_vertices, _indices);
    uint32_t matId = mScene.createMaterial(glm::float4(1.0));
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = mScene.createInstance(meshId, -1, transform);

    return ret;
}
} // namespace nevk

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

    for (size_t s = 0; s < shapes.size(); s++)
    {
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

    uint32_t meshId = mScene.createMesh(_vertices, _indices);
    uint32_t matId = mScene.createMaterial(glm::float4(1.0));
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = mScene.createInstance(meshId, -1, transform);

    return ret;
}
} // namespace nevk

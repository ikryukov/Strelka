#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <chrono>
#include <set>
#include <array>
#include <unordered_map>

#include "Vertex.h"

namespace nevk
{

class Model
{
private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

public:
    Model(){}

    void loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), MTL_PATH.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                int fv = shapes[s].mesh.num_face_vertices[f];
                for (size_t v = 0; v < fv; v++)
                {
                    Vertex vertex{};
                    auto idx = shapes[s].mesh.indices[index_offset + v];
                    vertex.pos = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    vertex.texCoord = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                    };

                    vertex.color = { attrib.colors[3 * idx.vertex_index + 0],
                        attrib.colors[3 * idx.vertex_index + 1],
                        attrib.colors[3 * idx.vertex_index + 2] };

                    vertex.ka = { materials[shapes[s].mesh.material_ids[f]].ambient[0],
                        materials[shapes[s].mesh.material_ids[f]].ambient[1],
                        materials[shapes[s].mesh.material_ids[f]].ambient[2]

                    };

                    vertex.kd = { materials[shapes[s].mesh.material_ids[f]].diffuse[0],
                        materials[shapes[s].mesh.material_ids[f]].diffuse[1],
                        materials[shapes[s].mesh.material_ids[f]].diffuse[2]

                    };

                    vertex.ks = { materials[shapes[s].mesh.material_ids[f]].specular[0],
                        materials[shapes[s].mesh.material_ids[f]].specular[1],
                        materials[shapes[s].mesh.material_ids[f]].specular[2]

                    };

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                        m_vertices.push_back(vertex);
                    }

                    m_indices.push_back(uniqueVertices[vertex]);
                }
                index_offset += fv;
            }
        }
    }

    std::vector<Vertex> getVertices()
    {
        return m_vertices;
    }

    std::vector<uint32_t> getIndices()
    {
        return m_indices;
    }

};
}





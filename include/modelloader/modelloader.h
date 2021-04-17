#pragma once

#include "scene/scene.h"
#include "texturemanager/texturemanager.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

namespace nevk
{

class Model
{
private:
    std::vector<Scene::Vertex> _vertices;
    std::vector<uint32_t> _indices;
    nevk::TextureManager* mTexManager;
    std::vector<Scene::Vertex> _verts;
    std::vector<uint32_t> _inds;
    std::vector<std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>>> _opaque_objects;
    std::vector<std::map<std::vector<uint32_t>, std::vector<Scene::Vertex>>> _transparent_objects;
    std::vector<uint32_t> _transparent_illums = { 4, 6, 7, 9 }; // info from MTL doc

public:
    explicit Model(nevk::TextureManager* texManager)
        : mTexManager(texManager){};

    bool loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene, glm::float3 camPosition);

    void sortMaterials(glm::float3 position);

    void parseVertsInds(std::vector<std::map<std::vector<uint32_t>,
                                                 std::vector<Scene::Vertex>>> vec);

    std::vector<Scene::Vertex> getVertices()
    {
        return _vertices;
    }

    std::vector<uint32_t> getIndices()
    {
        return _indices;
    }
};
} // namespace nevk

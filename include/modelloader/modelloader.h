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

public:
    explicit Model(nevk::TextureManager* texManager)
        : mTexManager(texManager){};

    bool loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene* mScene);

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

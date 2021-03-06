#pragma once

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

#include "scene/scene.h"
#include "texturemanager/texturemanager.h"

namespace nevk
{

class Model
{
private:
    std::vector<Scene::Vertex> _vertices;
    std::vector<uint32_t> _indices;
    nevk::TextureManager* mTexManager;

public:
    explicit Model(nevk::TextureManager* texManager) : mTexManager(texManager) {};

    bool loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene);

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

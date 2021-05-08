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
    nevk::TextureManager* mTexManager = nullptr;
    std::vector<uint32_t> _transparent_illums = { 4, 6, 7, 9 }; // info from MTL doc

public:
    explicit Model(nevk::TextureManager* texManager)
        : mTexManager(texManager){};

    bool loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene);

    // TODO: could be static
    void computeTangent(std::vector<Scene::Vertex>& _vertices,
                        const std::vector<uint32_t>& _indices) const;
};
} // namespace nevk

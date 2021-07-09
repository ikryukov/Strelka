#pragma once

#include "scene/scene.h"
#include "texturemanager/texturemanager.h"

#include <string>
#include <vector>

namespace nevk
{

class ModelLoader
{
private:
    nevk::TextureManager* mTexManager = nullptr;

public:
    explicit ModelLoader(nevk::TextureManager* texManager)
        : mTexManager(texManager){};

    bool loadModel(const std::string& MODEL_PATH, const std::string& MTL_PATH, nevk::Scene& mScene);

    bool loadModelGltf(const std::string& modelPath, nevk::Scene& mScene);

    // TODO: could be static
    void computeTangent(std::vector<Scene::Vertex>& _vertices,
                        const std::vector<uint32_t>& _indices) const;
};
} // namespace nevk

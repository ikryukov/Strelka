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

    bool loadModelGltf(const std::string& modelPath, nevk::Scene& mScene);

    // TODO: could be static
    void computeTangent(std::vector<Scene::Vertex>& _vertices,
                        const std::vector<uint32_t>& _indices) const;
};
} // namespace nevk

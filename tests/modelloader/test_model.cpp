#include <modelloader/modelloader.h>
#include <render/render.h>

#include <doctest.h>
#include <iostream>

const std::string MODELPATH = "misc/test_data/cube/Cube.gltf";

TEST_CASE("load model")
{
    Render r;
    r.MODEL_PATH = MODELPATH;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    nevk::Scene scene;
    mTexManager->initSamplers();
    nevk::ModelLoader model(mTexManager);
    bool loaded = model.loadModelGltf(MODELPATH,scene);

    CHECK(loaded == true);
    CHECK(scene.getIndices().size() == 36);
    CHECK(scene.getVertices().size() == 36);

    CHECK(mTexManager->textures.size() == 2);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);

    mTexManager->textureDestroy();
    r.cleanup();
}

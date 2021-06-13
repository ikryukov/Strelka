#include <modelloader/modelloader.h>
#include <render/render.h>

#include <doctest.h>
#include <iostream>

const std::string MODELPATH = "misc/test_data/cube.obj";
const std::string MTLPATH = "misc/test_data";
const std::string TEXPATH1 = "textures/brickwall.png";
const std::string TEXPATH2 = "textures/awesomeface.png";
const std::string TEXPATH3 = "textures/container.jpg";

TEST_CASE("load model")
{
    Render r;
    r.MODEL_PATH = MODELPATH;
    r.MTL_PATH = MTLPATH;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    nevk::Scene scene;

    nevk::ModelLoader model(mTexManager);
    bool loaded = model.loadModel(MODELPATH, MTLPATH, scene);

    CHECK(loaded == true);
    CHECK(scene.getIndices().size() == 36);
    CHECK(scene.getVertices().size() == 36);

    CHECK(mTexManager->textures.size() == 1);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);

    mTexManager->textureDestroy();
    r.cleanup();
}

TEST_CASE("load textures")
{
    Render r;
    r.initWindow();
    r.initVulkan();
    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    nevk::Scene mScene;

    int firstTex = mTexManager->loadTexture(TEXPATH1, MTLPATH);
    int secondTex = mTexManager->loadTexture(TEXPATH2, MTLPATH);
    int thirdTex = mTexManager->loadTexture(TEXPATH3, MTLPATH);
    int doubleFirstTex = mTexManager->loadTexture(TEXPATH1, MTLPATH);

    CHECK(firstTex == doubleFirstTex);
    CHECK(firstTex == 0);
    CHECK(secondTex == 1);
    CHECK(thirdTex == 2);

    CHECK(mTexManager->textures.size() == 3);
    CHECK(mTexManager->textures[0].texHeight == 600);
    CHECK(mTexManager->textures[0].texWidth == 600);
    CHECK(mTexManager->textures[1].texHeight == 512);
    CHECK(mTexManager->textures[1].texWidth == 512);
    CHECK(mTexManager->textures[2].texHeight == 512);
    CHECK(mTexManager->textures[2].texWidth == 512);

    mTexManager->textureDestroy();
    r.cleanup();
}

#include <modelloader/modelloader.h>
#include <render/render.h>

#include <doctest.h>
#include <iostream>

const std::string MODELPATH = "misc/cube.obj";
const std::string MTLPATH = "misc/";
const std::string TEXPATH1 = "textures/brickwall.png";
const std::string TEXPATH2 = "textures/awesomeface.png";
const std::string TEXPATH3 = "textures/container.jpg";

Render initVK()
{
    Render r;

    r.setWindow();
    r.setInstance();
    r.setDebugMessenger();
    r.setSurface();
    r.setPhysicalDevice();
    r.setLogicalDevice();
    r.setSwapChain();
    r.setDescriptorPool();
    r.setCommandPool();

    nevk::ResourceManager* mResManager = new nevk::ResourceManager(r.getDevice(), r.getPhysicalDevice(), r.getCurrentFrameData().cmdPool, r.getGraphicsQueue());
    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), mResManager);
    r.setTexManager(mTexManager);
    r.setResManager(mResManager);

    r.setImageViews();
    r.setCommandBuffers();
    r.setSyncObjects();

    return r;
}

TEST_CASE("load model")
{
    Render r = initVK();

    nevk::TextureManager* mTexManager = r.getTexManager();
    nevk::Scene mScene;

    nevk::Model model(mTexManager);
    bool loaded = model.loadModel(MODELPATH, MTLPATH, mScene);

    CHECK(model.getIndices().size() == 36);
    CHECK(model.getVertices().size() == 36);
    CHECK(loaded == true);

    CHECK(mTexManager->textures.size() == 2);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);
    CHECK(mTexManager->textures[1].texWidth == 600);
    CHECK(mTexManager->textures[1].texHeight == 600);
}

TEST_CASE("load textures")
{
    Render r = initVK();

    nevk::TextureManager* mTexManager = r.getTexManager();
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
}
#include <modelloader/modelloader.h>

#include <doctest.h>

const std::string MODEL_PATH = "misc/cube.obj";
const std::string MTL_PATH = "misc/";

TEST_CASE("load model")
{
    nevk::Scene mScene;
    nevk::Model model;
    bool loaded = model.loadModel(MODEL_PATH, MTL_PATH, mScene);

    CHECK(loaded == true);
}

TEST_CASE("check model data")
{
    nevk::Scene mScene;
    nevk::Model model;
    model.loadModel(MODEL_PATH, MTL_PATH, mScene);

    CHECK(model.getIndices().size() == 36);
    CHECK(model.getVertices().size() == 36);
}

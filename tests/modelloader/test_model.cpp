#include <modelloader/modelloader.h>

#include <doctest.h>

const std::string MODEL_PATH = "misc/cube.obj";
const std::string MTL_PATH = "misc/";

TEST_CASE("model test")
{
    auto* e = new nevk::Model();
    CHECK(e != nullptr);
}

TEST_CASE("load model")
{
    nevk::Scene mScene;
    nevk::Model model;
    model.loadModel(MODEL_PATH, MTL_PATH, mScene);

    CHECK(model.getIndices());
    CHECK(model.getVertices());
}

#include <shadermanager/ShaderManager.h>

#include <doctest.h>

using namespace nevk;

TEST_CASE("shader manager test")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    CHECK(sm != nullptr);
}

TEST_CASE("shader manager load")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    uint32_t pixelShaderId = sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    CHECK(pixelShaderId != -1);
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex) != -1);
}

TEST_CASE("shader manager compute")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    uint32_t compShaderId = sm->loadShader("../../shaders/test/test_comp_shader.hlsl", "computeMain", nevk::ShaderManager::Stage::eCompute);
    CHECK(compShaderId != -1);
    std::vector<ShaderManager::ResourceDesc> descs = sm->getResourcesDesc(compShaderId);
    CHECK(!descs.empty());
}

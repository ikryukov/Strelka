#include <shadermanager/ShaderManager.h>

#include <doctest.h>

#include <fstream>

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

TEST_CASE("shader manager compile from memory")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    std::ifstream fin("../../shaders/test/test_shader.hlsl");
    size_t size = fin.tellg();
    fin.seekg(0, std::ios::beg);

    std::string code = "";
    code.resize(size);

    fin.read(code.data(), size);

    uint32_t pixelShaderId = sm->loadShaderFromString(code.c_str(), "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    CHECK(pixelShaderId != -1);
}

TEST_CASE("shader manager compute")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    uint32_t compShaderId = sm->loadShader("../../shaders/test/test_comp_shader.hlsl", "computeMain", nevk::ShaderManager::Stage::eCompute);
    CHECK(compShaderId != -1);
    std::vector<ShaderManager::ResourceDesc> descs = sm->getResourcesDesc(compShaderId);
    CHECK(!descs.empty());
}

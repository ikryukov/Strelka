#include <shadermanager/ShaderManager.h>

#include <doctest.h>

#include <fstream>
#include <iostream>

using namespace oka;

TEST_CASE("shader manager test")
{
    oka::ShaderManager* sm = new oka::ShaderManager();
    CHECK(sm != nullptr);
}

TEST_CASE("shader manager load")
{
    oka::ShaderManager* sm = new oka::ShaderManager();
    uint32_t pixelShaderId = sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", oka::ShaderManager::Stage::ePixel);
    CHECK(pixelShaderId != -1);
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "vertexMain", oka::ShaderManager::Stage::eVertex) != -1);
}

TEST_CASE("shader manager compile from memory")
{
    oka::ShaderManager* sm = new oka::ShaderManager();
    std::ifstream fin("../../shaders/test/test_shader.hlsl");
    CHECK(fin);
    if (fin)
    {
        fin.seekg(0, std::ios::end);
        size_t size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        std::string code = "";
        code.resize(size);
        fin.read(code.data(), size);
        uint32_t pixelShaderId = sm->loadShaderFromString(code.c_str(), "fragmentMain", oka::ShaderManager::Stage::ePixel);
        CHECK(pixelShaderId != -1);
    }
}

TEST_CASE("shader manager compute")
{
    oka::ShaderManager* sm = new oka::ShaderManager();
    uint32_t compShaderId = sm->loadShader("../../shaders/test/test_comp_shader.hlsl", "computeMain", oka::ShaderManager::Stage::eCompute);
    CHECK(compShaderId != -1);
    std::vector<ShaderManager::ResourceDesc> descs = sm->getResourcesDesc(compShaderId);
    CHECK(!descs.empty());
}

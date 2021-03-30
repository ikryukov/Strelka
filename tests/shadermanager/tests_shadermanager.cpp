#include <shadermanager/ShaderManager.h>

#include <doctest.h>

TEST_CASE("shader manager test")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    CHECK(sm != nullptr);
}

TEST_CASE("shader manager load")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel) != -1);
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex) != -1);
}

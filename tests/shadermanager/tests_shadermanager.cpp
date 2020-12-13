#include <shadermanager/ShaderManager.h>

#include <doctest.h>

TEST_CASE("shader manager test")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    CHECK(sm->mSlangSession != nullptr);
}

TEST_CASE("shader manager load")
{
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", true) != false);
}

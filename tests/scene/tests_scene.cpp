#include <scene/scene.h>

#include <doctest.h>

TEST_CASE("scene test")
{
    nevk::Scene* e = new nevk::Scene();
    CHECK(e != nullptr);
}

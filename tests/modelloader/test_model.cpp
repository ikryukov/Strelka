#include <modelloader/modelloader.h>

#include <doctest.h>

TEST_CASE("model test")
{
    nevk::Model* e = new nevk::Model();
    CHECK(e != nullptr);
}

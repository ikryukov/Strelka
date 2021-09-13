#include <render/render.h>

#include <doctest.h>

using namespace nevk;

TEST_CASE("render test")
{
    Render* e = new Render();
    CHECK(e != nullptr);
}

#include <render/render.h>

#include <doctest.h>

TEST_CASE("render test")
{
    Render* e = new Render();
    CHECK(e != nullptr);
}

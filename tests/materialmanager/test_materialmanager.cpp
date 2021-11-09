#include "materialmanager.h"
#include <doctest.h>

using namespace nevk;

TEST_CASE("material manager test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);
}

TEST_CASE("init MDL test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string path = "external/mdl-sdk/macosx-x86-64/lib";
    bool res  = matmngr->initMDL(path.c_str());
    CHECK(res != 0);
}

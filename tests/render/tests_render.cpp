#include <render/ptrender.h>

#include <scene/scene.h>
#include <modelloader/modelloader.h>

#include <doctest.h>

namespace fs = std::filesystem;
using namespace nevk;


TEST_CASE("render test")
{
    PtRender* render = new PtRender();

    render->initVulkan();

    SharedContext& shared = render->getSharedContext();

    Scene scene;

    ModelLoader modelLoader(shared.mTextureManager);
    modelLoader.loadModelGltf("./misc/m4/minbox.gltf", scene);

    Scene::RectLightDesc desc{};
    desc.color = glm::float3(1.0f);
    desc.height = 0.4f;
    desc.width = 0.4f;
    desc.position = glm::float3(0, 1.1, 0.67);
    desc.orientation = glm::float3(179.68, 29.77, -89.97);
    desc.intensity = 160.0f;

    scene.createLight(desc);

    render->setScene(&scene);

    render->drawFrame();

    CHECK(render != nullptr);
}

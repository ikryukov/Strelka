#include <render/ptrender.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image_write.h>

#include <scene/scene.h>

#include <doctest.h>

namespace fs = std::filesystem;
using namespace nevk;


//TEST_CASE("render test")
//{
//    PtRender* render = new PtRender();
//
//    render->initVulkan();
//
//    SharedContext& shared = render->getSharedContext();
//
//    Scene scene;
//
//    ModelLoader modelLoader(shared.mTextureManager);
//    modelLoader.loadModelGltf("./misc/m4/minbox.gltf", scene);
//    //modelLoader.loadModelGltf("./misc/cornell_box/cornell_box.gltf", scene);
//
//    Scene::UniformLightDesc desc{};
//    desc.color = glm::float3(1.0f);
//    desc.height = 0.4f;
//    desc.width = 0.4f;
//    desc.position = glm::float3(0, 1.1, 0.67);
//    desc.orientation = glm::float3(179.68, 29.77, -89.97);
//    desc.intensity = 160.0f;
//
//    scene.createLight(desc);
//
//    render->setScene(&scene);
//
//    std::vector<float> data(800 * 600 * 4);
//    const uint8_t* pixels = (const uint8_t*) data.data();
//
//    render->drawFrame(pixels);
//
//    std::vector<uint8_t> saveData;
//    for (const float& p: data)
//    {
//        uint8_t v = p * 255.0f;
//        saveData.push_back(v);
//    }
//
//    stbi_write_png("res.bmp", 800, 600, 4, saveData.data(), 800 * 4);
//
//    CHECK(render != nullptr);
//}

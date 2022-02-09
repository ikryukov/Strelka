#include <pxr/pxr.h>
#include <pxr/base/gf/gamma.h>
#include <pxr/base/tf/stopwatch.h>
#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/imaging/hf/pluginDesc.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hio/image.h>
#include <pxr/imaging/hio/imageRegistry.h>
#include <pxr/imaging/hio/types.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

#include <algorithm>
#include <iostream>

#include "SimpleRenderTask.h"

#include <render/common.h>
#include <render/glfwrender.h>

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(
    _AppTokens,
    (HdNeVKDriver)
    (HdNeVKRendererPlugin));

HdRendererPluginHandle GetHdNeVKPlugin()
{
    HdRendererPluginRegistry& registry = HdRendererPluginRegistry::GetInstance();

    HfPluginDescVector pluginDescriptors;
    registry.GetPluginDescs(&pluginDescriptors);

    for (const HfPluginDesc& pluginDesc : pluginDescriptors)
    {
        const TfToken& pluginId = pluginDesc.id;

        if (pluginId != _AppTokens->HdNeVKRendererPlugin)
        {
            continue;
        }

        HdRendererPluginHandle plugin = registry.GetOrCreateRendererPlugin(pluginId);

        return plugin;
    }

    return HdRendererPluginHandle();
}

HdCamera* FindCamera(UsdStageRefPtr& stage, HdRenderIndex* renderIndex, std::string& settingsCameraPath)
{
    SdfPath cameraPath;

    if (!settingsCameraPath.empty())
    {
        cameraPath = SdfPath(settingsCameraPath);
    }
    else
    {
        UsdPrimRange primRange = stage->TraverseAll();
        for (auto prim = primRange.cbegin(); prim != primRange.cend(); prim++)
        {
            if (!prim->IsA<UsdGeomCamera>())
            {
                continue;
            }
            cameraPath = prim->GetPath();
            break;
        }
    }

    HdCamera* camera = (HdCamera*)dynamic_cast<HdCamera*>(renderIndex->GetSprim(HdTokens->camera, cameraPath));

    return camera;
}

int main(int argc, const char* argv[])
{
    // Init plugin.
    HdRendererPluginHandle pluginHandle = GetHdNeVKPlugin();

    if (!pluginHandle)
    {
        fprintf(stderr, "HdNeVK plugin not found!\n");
        return EXIT_FAILURE;
    }

    if (!pluginHandle->IsSupported())
    {
        fprintf(stderr, "HdNeVK plugin is not supported!\n");
        return EXIT_FAILURE;
    }
    
    HdDriverVector drivers;

    nevk::GLFWRender render;

    render.init(800, 600);

    nevk::SharedContext* ctx = &render.getSharedContext();

    HdDriver driver;
    driver.name = _AppTokens->HdNeVKDriver;
    driver.driver = VtValue(ctx);

    drivers.push_back(&driver);

    HdRenderDelegate* renderDelegate = pluginHandle->CreateRenderDelegate();
    TF_VERIFY(renderDelegate);
    renderDelegate->SetDrivers(drivers);

    // Handle cmdline args.
    // Load scene.
    TfStopwatch timerLoad;
    timerLoad.Start();

    //ArGetResolver().ConfigureResolverForAsset(settings.sceneFilePath);
    // std::string usdPath = "/Users/ilya/work/Kitchen_set/Kitchen_set.usd";
    // std::string usdPath = "./misc/glassCube.usda";
    std::string usdPath = "./misc/glassLens.usda";
    //std::string usdPath = "C:/work/Kitchen_set/Kitchen_set_cam.usd";

    UsdStageRefPtr stage = UsdStage::Open(usdPath.c_str());

    timerLoad.Stop();

    if (!stage)
    {
        fprintf(stderr, "Unable to open USD stage file.\n");
        return EXIT_FAILURE;
    }

    printf("USD scene loaded (%.3fs)\n", timerLoad.GetSeconds());
    fflush(stdout);

    // Print the up-axis
    std::cout << "Stage up-axis: " << UsdGeomGetStageUpAxis(stage) << std::endl;

    // Print the stage's linear units, or "meters per unit"
    std::cout << "Meters per unit: " << UsdGeomGetStageMetersPerUnit(stage) << std::endl;

    HdRenderIndex* renderIndex = HdRenderIndex::New(renderDelegate, HdDriverVector());
    TF_VERIFY(renderIndex);

    UsdImagingDelegate sceneDelegate(renderIndex, SdfPath::AbsoluteRootPath());
    sceneDelegate.Populate(stage->GetPseudoRoot());
    sceneDelegate.SetTime(0);
    sceneDelegate.SetRefineLevelFallback(4);

    double meterPerUnit = UsdGeomGetStageMetersPerUnit(stage);

    std::string cameraPath = "";

    HdCamera* camera = FindCamera(stage, renderIndex, cameraPath);
    if (!camera)
    {
        fprintf(stderr, "Camera not found!\n");
        return EXIT_FAILURE;
    }

    // Set up rendering context.
    uint32_t imageWidth = 800;
    uint32_t imageHeight = 600;

    HdRenderBuffer* renderBuffers[3];
    for (int i = 0; i < 3; ++i)
    {
        renderBuffers[i] = (HdRenderBuffer*)renderDelegate->CreateFallbackBprim(HdPrimTypeTokens->renderBuffer);
        renderBuffers[i]->Allocate(GfVec3i(imageWidth, imageHeight, 1), HdFormatFloat32Vec4, false);
    }
    
    CameraUtilFraming framing;
    framing.dataWindow = GfRect2i(GfVec2i(0, 0), GfVec2i(imageWidth, imageHeight));
    framing.displayWindow = GfRange2f(GfVec2f(0.0f, 0.0f), GfVec2f((float)imageWidth, (float)imageHeight));
    framing.pixelAspectRatio = 1.0f;

    std::pair<bool, CameraUtilConformWindowPolicy> overrideWindowPolicy(false, CameraUtilFit);

    TfTokenVector renderTags(1, HdRenderTagTokens->geometry);
    HdRprimCollection renderCollection(HdTokens->geometry, HdReprSelector(HdReprTokens->refined));
    HdRenderPassSharedPtr renderPass = renderDelegate->CreateRenderPass(renderIndex, renderCollection);

    std::shared_ptr<HdRenderPassState> renderPassState[3];
    std::shared_ptr<SimpleRenderTask> renderTasks[3];
    for (int i = 0; i < 3; ++i)
    {
        renderPassState[i] = std::make_shared<HdRenderPassState>();
        renderPassState[i]->SetCameraAndFraming(camera, framing, overrideWindowPolicy);
        HdRenderPassAovBindingVector aovBindings(1);
        aovBindings[0].aovName = HdAovTokens->color;
        aovBindings[0].renderBuffer = renderBuffers[i];

        renderPassState[i]->SetAovBindings(aovBindings);
        renderTasks[i] = std::make_shared<SimpleRenderTask>(renderPass, renderPassState[i], renderTags);    
    }
    
    // Perform rendering.
    TfStopwatch timerRender;
    timerRender.Start();

    HdEngine engine;

    uint64_t frameCount = 0;
    while (!render.windowShouldClose())
    {
        auto start = std::chrono::high_resolution_clock::now();
        HdTaskSharedPtrVector tasks;
        tasks.push_back(renderTasks[frameCount % 3]);
        sceneDelegate.SetTime(1.0f);
        
        render.pollEvents();

        render.onBeginFrame();
        engine.Execute(renderIndex, &tasks);
        nevk::Image* outputImage = renderBuffers[frameCount % 3]->GetResource(false).UncheckedGet<nevk::Image*>();
        render.drawFrame(outputImage);
        render.onEndFrame();
        
        auto finish = std::chrono::high_resolution_clock::now();
        double frameTime = std::chrono::duration<double, std::milli>(finish - start).count();
        ++frameCount;
    }

    //renderBuffer->Resolve();
    //TF_VERIFY(renderBuffer->IsConverged());

    timerRender.Stop();

    printf("Rendering finished (%.3fs)\n", timerRender.GetSeconds());
    fflush(stdout);

    // Gamma correction.
    //float* mappedMem = (float*)renderBuffer->Map();
    //TF_VERIFY(mappedMem != nullptr);

    //int pixelCount = renderBuffer->GetWidth() * renderBuffer->GetHeight();

    //for (int i = 0; i < pixelCount; i++)
    //{
    //    mappedMem[i * 4 + 0] = GfConvertLinearToDisplay(mappedMem[i * 4 + 0]);
    //    mappedMem[i * 4 + 1] = GfConvertLinearToDisplay(mappedMem[i * 4 + 1]);
    //    mappedMem[i * 4 + 2] = GfConvertLinearToDisplay(mappedMem[i * 4 + 2]);
    //}

    //// Write image to file.
    //TfStopwatch timerWrite;
    //timerWrite.Start();

    //std::string outputFilePath = "res.png";

    //HioImageSharedPtr image = HioImage::OpenForWriting(outputFilePath);

    //if (!image)
    //{
    //    fprintf(stderr, "Unable to open output file for writing!\n");
    //    return EXIT_FAILURE;
    //}

    //HioImage::StorageSpec storage;
    //storage.width = (int)renderBuffer->GetWidth();
    //storage.height = (int)renderBuffer->GetHeight();
    //storage.depth = (int)renderBuffer->GetDepth();
    //storage.format = HioFormat::HioFormatFloat32Vec4;
    //storage.flipped = false;
    //storage.data = mappedMem;

    //VtDictionary metadata;
    //image->Write(storage, metadata);

    //renderBuffer->Unmap();
    //timerWrite.Stop();

    //printf("Wrote image (%.3fs)\n", timerWrite.GetSeconds());
    //fflush(stdout);

    for (int i = 0; i < 3; ++i)
    {
        renderDelegate->DestroyBprim(renderBuffers[i]);
    }
    return EXIT_SUCCESS;
}

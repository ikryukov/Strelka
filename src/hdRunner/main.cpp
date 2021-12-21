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
#include <pxr/usdImaging/usdImaging/delegate.h>

#include <algorithm>

#include "SimpleRenderTask.h"

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(
  _AppTokens,
  (HdNeVKRendererPlugin)
);

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

    HdRenderDelegate* renderDelegate = pluginHandle->CreateRenderDelegate();
    TF_VERIFY(renderDelegate);

    // Handle cmdline args.
    // Load scene.
    TfStopwatch timerLoad;
    timerLoad.Start();

    //ArGetResolver().ConfigureResolverForAsset(settings.sceneFilePath);
    // std::string usdPath = "/Users/ilya/work/Kitchen_set/Kitchen_set.usd";
    std::string usdPath = "./misc/cornell.usdc";
    // std::string usdPath = "C:/work/Kitchen_set/Kitchen_set_cam.usd";

    UsdStageRefPtr stage = UsdStage::Open(usdPath.c_str());

    timerLoad.Stop();

    if (!stage)
    {
        fprintf(stderr, "Unable to open USD stage file.\n");
        return EXIT_FAILURE;
    }

    printf("USD scene loaded (%.3fs)\n", timerLoad.GetSeconds());
    fflush(stdout);

    HdRenderIndex* renderIndex = HdRenderIndex::New(renderDelegate, HdDriverVector());
    TF_VERIFY(renderIndex);

    UsdImagingDelegate sceneDelegate(renderIndex, SdfPath::AbsoluteRootPath());
    sceneDelegate.Populate(stage->GetPseudoRoot());
    sceneDelegate.SetTime(0);
    sceneDelegate.SetRefineLevelFallback(4);

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
    HdRenderBuffer* renderBuffer = (HdRenderBuffer*)renderDelegate->CreateFallbackBprim(HdPrimTypeTokens->renderBuffer);
    renderBuffer->Allocate(GfVec3i(imageWidth, imageHeight, 1), HdFormatFloat32Vec4, false);

    HdRenderPassAovBindingVector aovBindings(1);
    aovBindings[0].aovName = HdAovTokens->color;
    aovBindings[0].renderBuffer = renderBuffer;

    CameraUtilFraming framing;
    framing.dataWindow = GfRect2i(GfVec2i(0, 0), GfVec2i(imageWidth, imageHeight));
    framing.displayWindow = GfRange2f(GfVec2f(0.0f, 0.0f), GfVec2f((float)imageWidth, (float)imageHeight));
    framing.pixelAspectRatio = 1.0f;

    std::pair<bool, CameraUtilConformWindowPolicy> overrideWindowPolicy(false, CameraUtilFit);

    auto renderPassState = std::make_shared<HdRenderPassState>();
    renderPassState->SetCameraAndFraming(camera, framing, overrideWindowPolicy);
    renderPassState->SetAovBindings(aovBindings);

    HdRprimCollection renderCollection(HdTokens->geometry, HdReprSelector(HdReprTokens->refined));
    HdRenderPassSharedPtr renderPass = renderDelegate->CreateRenderPass(renderIndex, renderCollection);

    TfTokenVector renderTags(1, HdRenderTagTokens->geometry);
    auto renderTask = std::make_shared<SimpleRenderTask>(renderPass, renderPassState, renderTags);

    HdTaskSharedPtrVector tasks;
    tasks.push_back(renderTask);

    // Perform rendering.
    TfStopwatch timerRender;
    timerRender.Start();

    HdEngine engine;
    engine.Execute(renderIndex, &tasks);

    renderBuffer->Resolve();
    TF_VERIFY(renderBuffer->IsConverged());

    timerRender.Stop();

    printf("Rendering finished (%.3fs)\n", timerRender.GetSeconds());
    fflush(stdout);

    // Gamma correction.
    float* mappedMem = (float*)renderBuffer->Map();
    TF_VERIFY(mappedMem != nullptr);

    int pixelCount = renderBuffer->GetWidth() * renderBuffer->GetHeight();

    for (int i = 0; i < pixelCount; i++)
    {
        mappedMem[i * 4 + 0] = GfConvertLinearToDisplay(mappedMem[i * 4 + 0]);
        mappedMem[i * 4 + 1] = GfConvertLinearToDisplay(mappedMem[i * 4 + 1]);
        mappedMem[i * 4 + 2] = GfConvertLinearToDisplay(mappedMem[i * 4 + 2]);
    }

    // Write image to file.
    TfStopwatch timerWrite;
    timerWrite.Start();

    std::string outputFilePath = "res.png";

    HioImageSharedPtr image = HioImage::OpenForWriting(outputFilePath);

    if (!image)
    {
        fprintf(stderr, "Unable to open output file for writing!\n");
        return EXIT_FAILURE;
    }

    HioImage::StorageSpec storage;
    storage.width = (int)renderBuffer->GetWidth();
    storage.height = (int)renderBuffer->GetHeight();
    storage.depth = (int)renderBuffer->GetDepth();
    storage.format = HioFormat::HioFormatFloat32Vec4;
    storage.flipped = false;
    storage.data = mappedMem;

    VtDictionary metadata;
    image->Write(storage, metadata);

    renderBuffer->Unmap();
    timerWrite.Stop();

    printf("Wrote image (%.3fs)\n", timerWrite.GetSeconds());
    fflush(stdout);

    renderDelegate->DestroyBprim(renderBuffer);

    return EXIT_SUCCESS;
}

#include <pxr/pxr.h>
#include <pxr/base/gf/gamma.h>
#include <pxr/base/gf/rotation.h>
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
    (HdStrelkaDriver)
    (HdStrelkaRendererPlugin));

HdRendererPluginHandle GetHdStrelkaPlugin()
{
    HdRendererPluginRegistry& registry = HdRendererPluginRegistry::GetInstance();
    const TfToken& pluginId = _AppTokens->HdStrelkaRendererPlugin;
    HdRendererPluginHandle plugin = registry.GetOrCreateRendererPlugin(pluginId);
    return plugin;
}

HdCamera* FindCamera(UsdStageRefPtr& stage, HdRenderIndex* renderIndex, SdfPath& cameraPath)
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
    HdCamera* camera = (HdCamera*)dynamic_cast<HdCamera*>(renderIndex->GetSprim(HdTokens->camera, cameraPath));
    return camera;
}

class CameraController : public oka::InputHandler
{
    GfCamera mGfCam;
    GfQuatd mOrientation;
    GfVec3d mPosition;

    float rotationSpeed = 0.025f;
    float movementSpeed = 5.0f;

public:
    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool forward = false;
        bool back = false;
    } keys;
    struct MouseButtons
    {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

    GfVec2d mMousePos;

public:
    GfVec3d getFront()
    {
        return mOrientation.Transform(GfVec3d(0.0, 0.0, -1.0));
    }
    GfVec3d getUp()
    {
        return mOrientation.Transform(GfVec3d(0.0, 1.0, 0.0));
    }
    GfVec3d getRight()
    {
        return mOrientation.Transform(GfVec3d(1.0, 0.0, 0.0));
    }
    bool moving()
    {
        return keys.left || keys.right || keys.up || keys.down || keys.forward || keys.back || mouseButtons.right ||
               mouseButtons.left || mouseButtons.middle;
    }
    void update(double deltaTime)
    {
        if (moving())
        {
            const float moveSpeed = deltaTime * movementSpeed;
            if (keys.up)
                mPosition += getUp() * moveSpeed;
            if (keys.down)
                mPosition -= getUp() * moveSpeed;
            if (keys.left)
                mPosition -= getRight() * moveSpeed;
            if (keys.right)
                mPosition += getRight() * moveSpeed;
            if (keys.forward)
                mPosition += getFront() * moveSpeed;
            if (keys.back)
                mPosition -= getFront() * moveSpeed;
            updateViewMatrix();
        }
    }

    void rotate(double rightAngle, double upAngle)
    {
        GfRotation a(GfVec3d(1.0, 0.0, 0.0), upAngle * rotationSpeed);
        // GfRotation a(getRight(), upAngle * rotationSpeed);
        GfRotation b(GfVec3d(0.0, 1.0, 0.0), rightAngle * rotationSpeed);
        // GfRotation b(getUp(), rightAngle * rotationSpeed);
        // mOrientation = a.GetQuat() * mOrientation * b.GetQuat();
        mOrientation = mOrientation * (a * b).GetQuat();
        mOrientation.Normalize();
        updateViewMatrix();
    }

    void translate(GfVec3d delta)
    {
        mPosition += mOrientation.Transform(delta);
        updateViewMatrix();
    }

    void updateViewMatrix()
    {
        GfMatrix4d view(1.0);
        view.SetRotateOnly(mOrientation);
        view.SetTranslateOnly(mPosition);

        mGfCam.SetTransform(view);
    }

    GfCamera& getCamera()
    {
        return mGfCam;
    }

    CameraController(UsdGeomCamera& cam)
    {
        mGfCam = cam.GetCamera(0.0);
        GfMatrix4d xform = mGfCam.GetTransform();
        xform.Orthonormalize();
        mOrientation = xform.ExtractRotationQuat();
        mOrientation.Normalize();
        mPosition = xform.ExtractTranslation();
    }

    void keyCallback(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
    {
        const bool keyState = ((GLFW_REPEAT == action) || (GLFW_PRESS == action)) ? true : false;
        switch (key)
        {
        case GLFW_KEY_W: {
            keys.forward = keyState;
            break;
        }
        case GLFW_KEY_S: {
            keys.back = keyState;
            break;
        }
        case GLFW_KEY_A: {
            keys.left = keyState;
            break;
        }
        case GLFW_KEY_D: {
            keys.right = keyState;
            break;
        }
        case GLFW_KEY_Q: {
            keys.up = keyState;
            break;
        }
        case GLFW_KEY_E: {
            keys.down = keyState;
            break;
        }
        default:
            break;
        }
    }

    void mouseButtonCallback(int button, int action, [[maybe_unused]] int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                mouseButtons.right = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouseButtons.right = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                mouseButtons.left = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouseButtons.left = false;
            }
        }
    }

    void handleMouseMoveCallback([[maybe_unused]] double xpos, [[maybe_unused]] double ypos)
    {
        const float dx = mMousePos[0] - xpos;
        const float dy = mMousePos[1] - ypos;

        // ImGuiIO& io = ImGui::GetIO();
        // bool handled = io.WantCaptureMouse;
        // if (handled)
        //{
        //    camera.mousePos = glm::vec2((float)xpos, (float)ypos);
        //    return;
        //}

        if (mouseButtons.right)
        {
            rotate(dx, dy);
        }
        if (mouseButtons.left)
        {
            translate(GfVec3d(-0.0, 0.0, -dy * .005 * movementSpeed));
        }
        if (mouseButtons.middle)
        {
            translate(GfVec3d(-dx * 0.01, -dy * 0.01, 0.0f));
        }
        mMousePos[0] = xpos;
        mMousePos[1] = ypos;
    }
};

class RenderSurfaceController : public oka::ResizeHandler
{
    uint32_t imageWidth = 800;
    uint32_t imageHeight = 600;

public:

    void framebufferResize(int newWidth, int newHeight)
    {
    
    }
};

void setDefaultCamera(UsdGeomCamera& cam)
{
    std::vector<float> r0 = {0.1269921064376831, -0.9918715357780457, -0.007994583807885647, 0};
    std::vector<float> r1 = {0.6746357679367065, 0.08046140521764755, 0.7337523698806763, 0};
    std::vector<float> r2 = {-0.7271447777748108, -0.0985741913318634, 0.6793699860572815, 0};
    std::vector<float> r3 = {-2.080613613128662, -0.22476722300052643, 1.9320838451385498, 1};

    GfMatrix4d xform(r0, r1, r2, r3);
    GfCamera mGfCam{};

    mGfCam.SetTransform(xform);

    GfRange1f clippingRange = GfRange1f{0.1, 1000};
    mGfCam.SetClippingRange(clippingRange);
    mGfCam.SetVerticalAperture(20.25);
    mGfCam.SetVerticalApertureOffset(0);
    mGfCam.SetHorizontalAperture(36);
    mGfCam.SetHorizontalApertureOffset(0);
    mGfCam.SetFocalLength(50);

    GfCamera::Projection projection = GfCamera::Projection::Perspective;
    mGfCam.SetProjection(projection);

    cam.SetFromCamera(mGfCam, 0.0);
}

int main(int argc, const char* argv[])
{
    // Init plugin.
    HdRendererPluginHandle pluginHandle = GetHdStrelkaPlugin();

    if (!pluginHandle)
    {
        fprintf(stderr, "HdStrelka plugin not found!\n");
        return EXIT_FAILURE;
    }

    if (!pluginHandle->IsSupported())
    {
        fprintf(stderr, "HdStrelka plugin is not supported!\n");
        return EXIT_FAILURE;
    }

    HdDriverVector drivers;

    oka::GLFWRender render;

    render.init(800, 600);

    oka::SharedContext* ctx = &render.getSharedContext();

    ctx->mSettingsManager = new oka::SettingsManager();

    ctx->mSettingsManager->setAs<uint32_t>("render/pt/depth", 6);
    ctx->mSettingsManager->setAs<float>("render/pt/upscaleFactor", 0.5f);
    ctx->mSettingsManager->setAs<bool>("render/pt/enableUpscale", true);
    ctx->mSettingsManager->setAs<bool>("render/pt/enableAcc", true);

    HdDriver driver;
    driver.name = _AppTokens->HdStrelkaDriver;
    driver.driver = VtValue(ctx);

    drivers.push_back(&driver);

    HdRenderDelegate* renderDelegate = pluginHandle->CreateRenderDelegate();
    TF_VERIFY(renderDelegate);
    renderDelegate->SetDrivers(drivers);

    // Handle cmdline args.
    // Load scene.
    TfStopwatch timerLoad;
    timerLoad.Start();

    // ArGetResolver().ConfigureResolverForAsset(settings.sceneFilePath);
    // std::string usdPath = "/Users/ilya/work/Kitchen_set/Kitchen_set.usd";
    // std::string usdPath = "./misc/glassCube.usda";
    std::string usdPath = "./misc/glassLens.usda";
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

    // Print the up-axis
    std::cout << "Stage up-axis: " << UsdGeomGetStageUpAxis(stage) << std::endl;

    // Print the stage's linear units, or "meters per unit"
    std::cout << "Meters per unit: " << UsdGeomGetStageMetersPerUnit(stage) << std::endl;

    // Init default camera
    SdfPath cameraPath = SdfPath("/defaultCamera");
    UsdGeomCamera cam = UsdGeomCamera::Define(stage, cameraPath);
    setDefaultCamera(cam);

    HdRenderIndex* renderIndex = HdRenderIndex::New(renderDelegate, HdDriverVector());
    TF_VERIFY(renderIndex);

    UsdImagingDelegate sceneDelegate(renderIndex, SdfPath::AbsoluteRootPath());
    sceneDelegate.Populate(stage->GetPseudoRoot());
    sceneDelegate.SetTime(0);
    sceneDelegate.SetRefineLevelFallback(4);

    double meterPerUnit = UsdGeomGetStageMetersPerUnit(stage);

    // Init camera from scene
    cameraPath = SdfPath::EmptyPath();
    HdCamera* camera = FindCamera(stage, renderIndex, cameraPath);
    cam = UsdGeomCamera::Get(stage, cameraPath);
    CameraController cameraController(cam);

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
    render.setInputHandler(&cameraController);

    uint64_t frameCount = 0;
    while (!render.windowShouldClose())
    {
        auto start = std::chrono::high_resolution_clock::now();
        HdTaskSharedPtrVector tasks;
        tasks.push_back(renderTasks[frameCount % 3]);
        sceneDelegate.SetTime(1.0f);

        render.pollEvents();

        static auto prevTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        const double deltaTime = std::chrono::duration<double, std::milli>(currentTime - prevTime).count() / 1000.0;
        cameraController.update(deltaTime);
        prevTime = currentTime;

        cam.SetFromCamera(cameraController.getCamera(), 0.0);

        render.onBeginFrame();
        engine.Execute(renderIndex, &tasks); // main path tracing rendering in fixed render resolution
        oka::Image* outputImage = renderBuffers[frameCount % 3]->GetResource(false).UncheckedGet<oka::Image*>();
        render.drawFrame(outputImage); // blit rendered image to swapchain
        render.drawUI(); // render ui to swapchain image in window resolution
        render.onEndFrame(); // submit command buffer and present

        auto finish = std::chrono::high_resolution_clock::now();
        double frameTime = std::chrono::duration<double, std::milli>(finish - start).count();
        render.setWindowTitle((std::string("Strelka") + " [" + std::to_string(frameTime) + " ms]").c_str());
        ++frameCount;
    }

    // renderBuffer->Resolve();
    // TF_VERIFY(renderBuffer->IsConverged());

    timerRender.Stop();

    render.destroy();

    printf("Rendering finished (%.3fs)\n", timerRender.GetSeconds());
    fflush(stdout);

    // Gamma correction.
    // float* mappedMem = (float*)renderBuffer->Map();
    // TF_VERIFY(mappedMem != nullptr);

    // int pixelCount = renderBuffer->GetWidth() * renderBuffer->GetHeight();

    // for (int i = 0; i < pixelCount; i++)
    //{
    //    mappedMem[i * 4 + 0] = GfConvertLinearToDisplay(mappedMem[i * 4 + 0]);
    //    mappedMem[i * 4 + 1] = GfConvertLinearToDisplay(mappedMem[i * 4 + 1]);
    //    mappedMem[i * 4 + 2] = GfConvertLinearToDisplay(mappedMem[i * 4 + 2]);
    //}

    //// Write image to file.
    // TfStopwatch timerWrite;
    // timerWrite.Start();

    // std::string outputFilePath = "res.png";

    // HioImageSharedPtr image = HioImage::OpenForWriting(outputFilePath);

    // if (!image)
    //{
    //    fprintf(stderr, "Unable to open output file for writing!\n");
    //    return EXIT_FAILURE;
    //}

    // HioImage::StorageSpec storage;
    // storage.width = (int)renderBuffer->GetWidth();
    // storage.height = (int)renderBuffer->GetHeight();
    // storage.depth = (int)renderBuffer->GetDepth();
    // storage.format = HioFormat::HioFormatFloat32Vec4;
    // storage.flipped = false;
    // storage.data = mappedMem;

    // VtDictionary metadata;
    // image->Write(storage, metadata);

    // renderBuffer->Unmap();
    // timerWrite.Stop();

    // printf("Wrote image (%.3fs)\n", timerWrite.GetSeconds());
    // fflush(stdout);

    for (int i = 0; i < 3; ++i)
    {
        renderDelegate->DestroyBprim(renderBuffers[i]);
    }
    return EXIT_SUCCESS;
}

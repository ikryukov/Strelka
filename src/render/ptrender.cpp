#include "ptrender.h"

//#include "debugUtils.h"
#include "instanceconstants.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

// profiler
//#include "Tracy.hpp"

namespace fs = std::filesystem;
const uint32_t MAX_LIGHT_COUNT = 100;

using namespace nevk;

void PtRender::init()
{
    VkRender::initVulkan();

    mView[0] = createView(800, 600, 8);

    mDebugView = new DebugView(mSharedCtx);
    mDebugView->initialize();

    mTonemap = new Tonemap(mSharedCtx);
    mTonemap->initialize();

    mUpscalePass = new UpscalePass(mSharedCtx);
    mUpscalePass->initialize();

    mReductionPass = new ReductionPass(mSharedCtx);
    mReductionPass->initialize();

    mMaterialManager = new MaterialManager();

    const char* envUSDPath = std::getenv("USD_PATH");
    if (!envUSDPath)
    {
        printf("Please, set USD_PATH variable\n");
        assert(0);
    }
    const std::string usdMdlLibPath = std::string(envUSDPath) + "/mdl";

    const char* paths[4] = { "./misc/test_data/mtlx", "./misc/test_data/mdl/", 
        "./misc/test_data/mdl/resources/", usdMdlLibPath.c_str() };
    bool res = mMaterialManager->addMdlSearchPath(paths, 4);
    if (!res)
    {
        // failed to load MDL
        return;
    }

    std::vector<MaterialManager::CompiledMaterial*> materials;
    for (uint32_t i = 0; i < mScene->materialsCode.size(); ++i)
    {
        if (mScene->mMaterials[i].isMdl)
        {
            MaterialManager::Module* mdlModule = mMaterialManager->createModule(mScene->materialsCode[i].file.c_str());
            assert(mdlModule);
            MaterialManager::MaterialInstance* materialInst = mMaterialManager->createMaterialInstance(mdlModule, mScene->materialsCode[i].name.c_str());
            assert(materialInst);
            MaterialManager::CompiledMaterial* materialComp = mMaterialManager->compileMaterial(materialInst);
            assert(materialComp);
            materials.push_back(materialComp);
        }
        else
        {
            MaterialManager::Module* mdlModule = mMaterialManager->createMtlxModule(mScene->materialsCode[i].code.c_str());
            assert(mdlModule);
            MaterialManager::MaterialInstance* materialInst = mMaterialManager->createMaterialInstance(mdlModule, "");
            assert(materialInst);
            MaterialManager::CompiledMaterial* materialComp = mMaterialManager->compileMaterial(materialInst);
            assert(materialComp);
            materials.push_back(materialComp);
        }
    }

    const fs::path cwd = fs::current_path();
    std::ifstream pt(cwd.string() + "/shaders/pathtracerMdl.hlsl");
    std::stringstream ptcode;
    ptcode << pt.rdbuf();

    assert(materials.size() != 0);
    const MaterialManager::TargetCode* mdlTargetCode = mMaterialManager->generateTargetCode(materials);
    const char* hlsl = mMaterialManager->getShaderCode(mdlTargetCode);

    mCurrentSceneRenderData = new SceneRenderData(mSharedCtx.mResManager);

    mCurrentSceneRenderData->mMaterialTargetCode = mdlTargetCode;

    std::string newPTCode = std::string(hlsl) + "\n" + ptcode.str();

    mPathTracer = new PathTracer(mSharedCtx, newPTCode);
    mPathTracer->initialize();

    mAccumulationPathTracer = new Accumulation(mSharedCtx);
    mAccumulationPathTracer->initialize();

    TextureManager::TextureSamplerDesc defSamplerDesc{ VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
    mSharedCtx.mTextureManager->createTextureSampler(defSamplerDesc);

    createGbufferPass();
}

void PtRender::cleanup()
{
    mGbufferPass.onDestroy();

    delete mView[0];
    delete mView[1];
    delete mView[2];

    delete mTonemap;
    delete mReductionPass;
    delete mUpscalePass;
    delete mDebugView;
    delete mPathTracer;
    delete mAccumulationPathTracer;

    delete mDefaultSceneRenderData;
    if (mCurrentSceneRenderData != mDefaultSceneRenderData)
    {
        delete mCurrentSceneRenderData;
    }
}

PtRender::ViewData* PtRender::createView(uint32_t width, uint32_t height, uint32_t spp)
{
    assert(mSharedCtx.mResManager);
    assert(mSharedCtx.mTextureManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    TextureManager* texManager = mSharedCtx.mTextureManager;
    ViewData* view = new ViewData();
    view->spp = spp;
    view->finalWidth = width;
    view->finalHeight = height;
    view->renderWidth = (uint32_t)(width * 1.0f); // mRenderConfig.upscaleFactor
    view->renderHeight = (uint32_t)(height * 1.0f); // mRenderConfig.upscaleFactor
    view->mResManager = resManager;
    view->gbuffer = createGbuffer(view->renderWidth, view->renderHeight);
    view->prevDepth = resManager->createImage(view->renderWidth, view->renderHeight, view->gbuffer->depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Prev depth");
    texManager->transitionImageLayout(resManager->getVkImage(view->prevDepth), view->gbuffer->depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    resManager->setImageLayout(view->prevDepth, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    view->textureDebugViewImage = resManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                          VK_IMAGE_TILING_OPTIMAL,
                                                          VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "DebugView result");
    texManager->transitionImageLayout(resManager->getVkImage(view->textureDebugViewImage), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    resManager->setImageLayout(view->textureDebugViewImage, VK_IMAGE_LAYOUT_GENERAL);

    view->textureTonemapImage = resManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                        VK_IMAGE_TILING_OPTIMAL,
                                                        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Tonemap result");
    view->textureUpscaleImage = resManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                        VK_IMAGE_TILING_OPTIMAL,
                                                        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Upscale Output");
    texManager->transitionImageLayout(resManager->getVkImage(view->textureUpscaleImage), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    resManager->setImageLayout(view->textureUpscaleImage, VK_IMAGE_LAYOUT_GENERAL);

    const size_t compositingBufferSize = view->renderWidth * view->renderHeight * 3 * sizeof(float); // 3 - rgb
    const size_t sampleBufferSize = compositingBufferSize * spp;
    view->mSampleBuffer = resManager->createBuffer(sampleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Sample buffer");
    view->mCompositingBuffer = resManager->createBuffer(compositingBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Compositing buffer");

    view->mPathTracerImage = resManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                     VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Path Tracer Output");
    for (int i = 0; i < 2; ++i)
    {
        const std::string imageName = "Accumulation Image: " + std::to_string(i);
        view->mAccumulationPathTracerImage[i] = resManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                                        VK_IMAGE_TILING_OPTIMAL,
                                                                        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageName.c_str());
        texManager->transitionImageLayout(resManager->getVkImage(view->mAccumulationPathTracerImage[i]), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        resManager->setImageLayout(view->mAccumulationPathTracerImage[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return view;
}

GBuffer* PtRender::createGbuffer(uint32_t width, uint32_t height)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    assert(resManager);
    GBuffer* res = new GBuffer();
    res->mResManager = resManager;
    res->width = width;
    res->height = height;
    // Depth
    res->depthFormat = findDepthFormat();
    res->depth = resManager->createImage(width, height, res->depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "depth");
    // Normals
    res->normal = resManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "normal");
    // Tangent
    res->tangent = resManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "tangent");
    // wPos
    res->wPos = resManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "wPos");
    // UV
    res->uv = resManager->createImage(width, height, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "UV");
    // InstId
    res->instId = resManager->createImage(width, height, VK_FORMAT_R32_SINT, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "instId");
    // Motion
    res->motion = resManager->createImage(width, height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Motion");
    // Debug
    res->debug = resManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Debug");
    return res;
}

void PtRender::createGbufferPass()
{
    uint32_t vertId = mSharedCtx.mShaderManager->loadShader("shaders/gbuffer.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    uint32_t fragId = mSharedCtx.mShaderManager->loadShader("shaders/gbuffer.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    const char* vertShaderCode = nullptr;
    uint32_t vertShaderCodeSize = 0;
    const char* fragShaderCode = nullptr;
    uint32_t fragShaderCodeSize = 0;
    mSharedCtx.mShaderManager->getShaderCode(vertId, vertShaderCode, vertShaderCodeSize);
    mSharedCtx.mShaderManager->getShaderCode(fragId, fragShaderCode, fragShaderCodeSize);

    mGbufferPass.setTextureSamplers(mSharedCtx.mTextureManager->texSamplers);
    assert(mView[0]);
    mGbufferPass.init(mDevice, enableValidationLayers, vertShaderCode, vertShaderCodeSize, fragShaderCode, fragShaderCodeSize,
                      mSharedCtx.mDescriptorPool, mSharedCtx.mResManager, mView[0]->gbuffer);
    mGbufferPass.createFrameBuffers(*mView[0]->gbuffer, 0);
}

void PtRender::createVertexBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    std::vector<nevk::Scene::Vertex>& sceneVertices = scene.getVertices();
    VkDeviceSize bufferSize = sizeof(nevk::Scene::Vertex) * sceneVertices.size();
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneVertices.data(), (size_t)bufferSize);
    mCurrentSceneRenderData->mVertexBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "VB");
    resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mVertexBuffer), bufferSize);
    resManager->destroyBuffer(stagingBuffer);
}

void PtRender::createMaterialBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    std::vector<Material>& sceneMaterials = scene.getMaterials();

    VkDeviceSize bufferSize = sizeof(Material) * (sceneMaterials.size() + MAX_LIGHT_COUNT); // Reserve extra for lights material
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneMaterials.data(), sceneMaterials.size() * sizeof(Material));
    mCurrentSceneRenderData->mMaterialBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Materials");
    resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer), bufferSize);
    resManager->destroyBuffer(stagingBuffer);
}

void PtRender::createLightsBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    std::vector<nevk::Scene::Light>& sceneLights = scene.getLights();

    VkDeviceSize bufferSize = sizeof(nevk::Scene::Light) * MAX_LIGHT_COUNT;
    mCurrentSceneRenderData->mLightsBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Lights");

    if (!sceneLights.empty())
    {
        Buffer* stagingBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
        memcpy(stagingBufferMemory, sceneLights.data(), sceneLights.size() * sizeof(nevk::Scene::Light));
        resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mLightsBuffer), bufferSize);
        resManager->destroyBuffer(stagingBuffer);
    }
}

void PtRender::createBvhBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    const std::vector<Scene::Vertex>& vertices = scene.getVertices();
    const std::vector<uint32_t>& indices = scene.getIndices();
    const std::vector<Instance>& instances = scene.getInstances();
    const std::vector<Mesh>& meshes = scene.getMeshes();

    std::vector<BVHInputPosition> positions;
    positions.reserve(indices.size());
    uint32_t currInstId = 0;
    for (const Instance& currInstance : instances)
    {
        const uint32_t currentMeshId = currInstance.mMeshId;
        const Mesh& mesh = meshes[currentMeshId];
        const uint32_t indexOffset = mesh.mIndex;
        const uint32_t indexCount = mesh.mCount;
        assert(indexCount % 3 == 0);
        const uint32_t triangleCount = indexCount / 3;
        const glm::float4x4 m = currInstance.transform;

        for (uint32_t i = 0; i < triangleCount; ++i)
        {
            uint32_t i0 = indices[indexOffset + i * 3 + 0];
            uint32_t i1 = indices[indexOffset + i * 3 + 1];
            uint32_t i2 = indices[indexOffset + i * 3 + 2];

            glm::float3 v0 = m * glm::float4(vertices[i0].pos, 1.0);
            glm::float3 v1 = m * glm::float4(vertices[i1].pos, 1.0);
            glm::float3 v2 = m * glm::float4(vertices[i2].pos, 1.0);

            BVHInputPosition p0;
            p0.pos = v0;
            p0.instId = currInstId;
            p0.primId = i;
            positions.push_back(p0);

            BVHInputPosition p1;
            p1.pos = v1;
            p1.instId = currInstId;
            p1.primId = i;
            positions.push_back(p1);

            BVHInputPosition p2;
            p2.pos = v2;
            p2.instId = currInstId;
            p2.primId = i;
            positions.push_back(p2);
        }
        ++currInstId;
    }

    BVH sceneBvh = mBvhBuilder.build(positions);
    {
        VkDeviceSize bufferSize = sizeof(BVHNode) * sceneBvh.nodes.size();
        if (bufferSize == 0)
        {
            return;
        }
        Buffer* stagingBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
        memcpy(stagingBufferMemory, sceneBvh.nodes.data(), (size_t)bufferSize);
        mCurrentSceneRenderData->mBvhNodeBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "BVH");
        resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mBvhNodeBuffer), bufferSize);
        resManager->destroyBuffer(stagingBuffer);
    }
}

void PtRender::createIndexBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    std::vector<uint32_t>& sceneIndices = scene.getIndices();
    mCurrentSceneRenderData->mIndicesCount = (uint32_t)sceneIndices.size();
    VkDeviceSize bufferSize = sizeof(uint32_t) * sceneIndices.size();
    if (bufferSize == 0)
    {
        return;
    }

    Buffer* stagingBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneIndices.data(), (size_t)bufferSize);
    mCurrentSceneRenderData->mIndexBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IB");
    resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mIndexBuffer), bufferSize);
    resManager->destroyBuffer(stagingBuffer);
}

void PtRender::createInstanceBuffer(nevk::Scene& scene)
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    const std::vector<Mesh>& meshes = scene.getMeshes();
    const std::vector<nevk::Instance>& sceneInstances = scene.getInstances();
    mCurrentSceneRenderData->mInstanceCount = (uint32_t)sceneInstances.size();
    VkDeviceSize bufferSize = sizeof(InstanceConstants) * (sceneInstances.size() + MAX_LIGHT_COUNT); // Reserve some extra space for lights
    if (bufferSize == 0)
    {
        return;
    }
    std::vector<InstanceConstants> instanceConsts;
    instanceConsts.resize(sceneInstances.size());
    for (int i = 0; i < sceneInstances.size(); ++i)
    {
        instanceConsts[i].materialId = sceneInstances[i].mMaterialId;
        instanceConsts[i].objectToWorld = sceneInstances[i].transform;
        instanceConsts[i].worldToObject = glm::inverse(sceneInstances[i].transform);
        instanceConsts[i].normalMatrix = glm::inverse(glm::transpose(sceneInstances[i].transform));

        const uint32_t currentMeshId = sceneInstances[i].mMeshId;
        instanceConsts[i].indexOffset = meshes[currentMeshId].mIndex;
        instanceConsts[i].indexCount = meshes[currentMeshId].mCount;
    }

    Buffer* stagingBuffer = resManager->createBuffer(sizeof(InstanceConstants) * sceneInstances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, instanceConsts.data(), sizeof(InstanceConstants) * sceneInstances.size());
    mCurrentSceneRenderData->mInstanceBuffer = resManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Instance consts");
    resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer), sizeof(InstanceConstants) * sceneInstances.size());
    resManager->destroyBuffer(stagingBuffer);
}

void nevk::PtRender::createMdlBuffers()
{
    assert(mSharedCtx.mResManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    const MaterialManager::TargetCode* code = mCurrentSceneRenderData->mMaterialTargetCode;
    mMaterialManager->getArgBufferData(code);

    const uint32_t argSize = mMaterialManager->getArgBufferSize(code);
    const uint32_t roSize = mMaterialManager->getReadOnlyBlockSize(code);
    const uint32_t infoSize = mMaterialManager->getResourceInfoSize(code);
    const uint32_t mdlMaterialSize = mMaterialManager->getMdlMaterialSize(code);

    VkDeviceSize stagingSize = std::max(std::max(roSize, mdlMaterialSize), std::max(argSize, infoSize));

    Buffer* stagingBuffer = resManager->createBuffer(stagingSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Staging MDL");
    void* stagingBufferMemory = resManager->getMappedMemory(stagingBuffer);

    auto createGpuBuffer = [&](Buffer*& dest, const uint8_t* src, uint32_t size, const char* name) {
        memcpy(stagingBufferMemory, src, size);
        dest = resManager->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name);
        resManager->copyBuffer(resManager->getVkBuffer(stagingBuffer), resManager->getVkBuffer(dest), size);
    };

    createGpuBuffer(mCurrentSceneRenderData->mMdlArgBuffer, mMaterialManager->getArgBufferData(code), argSize, "MDL args");
    createGpuBuffer(mCurrentSceneRenderData->mMdlInfoBuffer, mMaterialManager->getResourceInfoData(code), infoSize, "MDL info");
    createGpuBuffer(mCurrentSceneRenderData->mMdlRoBuffer, mMaterialManager->getReadOnlyBlockData(code), roSize, "MDL read only");
    createGpuBuffer(mCurrentSceneRenderData->mMdlMaterialBuffer, mMaterialManager->getMdlMaterialData(code), mdlMaterialSize, "MDL mdl material");

    resManager->destroyBuffer(stagingBuffer);
}

void PtRender::drawFrame(const uint8_t* outPixels)
{
    assert(mSharedCtx.mResManager);
    assert(mSharedCtx.mTextureManager);
    ResourceManager* resManager = mSharedCtx.mResManager;
    TextureManager* texManager = mSharedCtx.mTextureManager;

    assert(mScene);
    // ZoneScoped;

    FrameData& currFrame = getCurrentFrameData();
    const uint32_t imageIndex = 0;
    const uint64_t frameIndex = 0;

    createVertexBuffer(*mScene);
    createIndexBuffer(*mScene);
    createInstanceBuffer(*mScene);
    createLightsBuffer(*mScene);
    createMaterialBuffer(*mScene);
    createMdlBuffers();
    createBvhBuffer(*mScene); // need to update descriptors after it

    ViewData* currView = mView[imageIndex];
    uint32_t renderWidth = currView->renderWidth;
    uint32_t renderHeight = currView->renderHeight;
    uint32_t finalWidth = currView->finalWidth;
    uint32_t finalHeight = currView->finalHeight;

    mScene->updateCamerasParams(renderWidth, renderHeight);

    Camera& cam = mScene->getCamera(getActiveCameraIndex());
    cam.updateViewMatrix();

    cam.prevMatrices = cam.matrices; // TODO:

    mGbufferPass.onResize(currView->gbuffer, 0);
    mGbufferPass.updateUniformBuffer(imageIndex, *mScene, getActiveCameraIndex());

    VkCommandBuffer& cmd = getFrameData(imageIndex).cmdBuffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);

    mGbufferPass.setTextureImageView(texManager->textureImageView);
    mGbufferPass.setTextureSamplers(texManager->texSamplers);
    mGbufferPass.setMaterialBuffer(resManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer));
    mGbufferPass.setInstanceBuffer(resManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer));

    mGbufferPass.record(cmd, resManager->getVkBuffer(mCurrentSceneRenderData->mVertexBuffer), resManager->getVkBuffer(mCurrentSceneRenderData->mIndexBuffer), *mScene, renderWidth, renderHeight, imageIndex, getActiveCameraIndex());

    const GBuffer& gbuffer = *currView->gbuffer;

    // barriers
    {
        recordImageBarrier(cmd, gbuffer.wPos, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.normal, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.tangent, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.uv, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.instId, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.motion, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordImageBarrier(cmd, gbuffer.depth, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        recordImageBarrier(cmd, gbuffer.debug, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    Image* finalPathTracerImage = currView->mPathTracerImage;
    Image* finalImage = nullptr;

    // Path Tracer
    for (int i = 0; i < 20; ++i)
    {
        PathTracerDesc ptDesc{};
        // desc.result = mView[imageIndex]->mPathTracerImage;
        ptDesc.gbuffer = currView->gbuffer;
        ptDesc.bvhNodes = mCurrentSceneRenderData->mBvhNodeBuffer;
        ptDesc.vb = mCurrentSceneRenderData->mVertexBuffer;
        ptDesc.ib = mCurrentSceneRenderData->mIndexBuffer;
        ptDesc.instanceConst = mCurrentSceneRenderData->mInstanceBuffer;
        ptDesc.lights = mCurrentSceneRenderData->mLightsBuffer;
        ptDesc.materials = mCurrentSceneRenderData->mMaterialBuffer;
        ptDesc.matSampler = texManager->mMdlSampler;
        ptDesc.matTextures = texManager->textureImages;

        ptDesc.sampleBuffer = currView->mSampleBuffer;
        // desc.compositingBuffer = currView->mCompositingBuffer;

        ptDesc.mdl_argument_block = mCurrentSceneRenderData->mMdlArgBuffer;
        ptDesc.mdl_ro_data_segment = mCurrentSceneRenderData->mMdlRoBuffer;
        ptDesc.mdl_resource_infos = mCurrentSceneRenderData->mMdlInfoBuffer;
        ptDesc.mdl_mdlMaterial = mCurrentSceneRenderData->mMdlMaterialBuffer;

        PathTracerParam& pathTracerParam = ptDesc.constants;
        pathTracerParam.dimension = glm::int2(renderWidth, renderHeight);
        pathTracerParam.frameNumber = (uint32_t)mFrameNumber;
        pathTracerParam.maxDepth = 8;
        pathTracerParam.debug = (uint32_t)(mScene->mDebugViewSettings == Scene::DebugView::ePTDebug);
        pathTracerParam.camPos = glm::float4(cam.getPosition(), 1.0f);
        pathTracerParam.viewToWorld = glm::inverse(cam.matrices.view);
        pathTracerParam.worldToView = cam.matrices.view; //
        // pathTracerParam.clipToView = glm::inverse(cam.matrices.perspective);
        pathTracerParam.clipToView = cam.matrices.invPerspective;
        pathTracerParam.viewToClip = cam.matrices.perspective; //
        pathTracerParam.len = (int)0;
        pathTracerParam.spp = currView->spp;
        pathTracerParam.iteration = i;
        pathTracerParam.numLights = (uint32_t)mScene->getLights().size();
        pathTracerParam.invDimension.x = 1.0f / (float)renderWidth;
        pathTracerParam.invDimension.y = 1.0f / (float)renderHeight;
        recordBufferBarrier(cmd, currView->mSampleBuffer, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        mPathTracer->execute(cmd, ptDesc, renderWidth * renderHeight * pathTracerParam.spp, 1, frameIndex);

        ReductionDesc reductionDesc{};
        reductionDesc.constants = ptDesc.constants;
        reductionDesc.result = currView->mPathTracerImage;
        reductionDesc.sampleBuffer = currView->mSampleBuffer;

        // buffer barrier
        recordBufferBarrier(cmd, currView->mSampleBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        // image barrier
        recordImageBarrier(cmd, currView->mPathTracerImage, VK_IMAGE_LAYOUT_GENERAL,
                           VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        mReductionPass->execute(cmd, reductionDesc, renderWidth * renderHeight, 1, frameIndex);

        recordImageBarrier(cmd, currView->mPathTracerImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        finalPathTracerImage = currView->mPathTracerImage;

        {
            // Accumulation pass
            Image* accHist = mView[imageIndex]->mAccumulationPathTracerImage[i % 2];
            Image* accOut = mView[imageIndex]->mAccumulationPathTracerImage[(i + 1) % 2];
            
            AccumulationDesc accDesc{};
            AccumulationParam& accParam = accDesc.constants;
            accParam.alpha = 0.1f;
            accParam.dimension = glm::int2(renderWidth, renderHeight);
            // glm::double4x4 persp = cam.prevMatrices.perspective;
            // accParam.prevClipToView = glm::inverse(persp);
            accParam.prevClipToView = cam.prevMatrices.invPerspective;
            accParam.prevViewToClip = cam.prevMatrices.perspective;
            accParam.prevWorldToView = cam.prevMatrices.view;
            glm::double4x4 view = cam.prevMatrices.view;
            accParam.prevViewToWorld = glm::inverse(view);
            // debug
            // accParam.clipToView = glm::inverse(cam.matrices.perspective);
            accParam.clipToView = cam.matrices.invPerspective;
            accParam.viewToWorld = glm::inverse(cam.matrices.view);
            accParam.isPt = 1;
            currView->mPtIteration = i; // TODO: need to recalc properly
            accParam.iteration = currView->mPtIteration;

            accDesc.input4 = finalPathTracerImage;
            accDesc.history4 = accHist;
            accDesc.output4 = accOut;
            accDesc.wpos = gbuffer.wPos;
            accDesc.currDepth = gbuffer.depth;
            accDesc.motion = gbuffer.motion;
            accDesc.prevDepth = gbuffer.depth; //TODO: fix

            recordImageBarrier(cmd, accOut, VK_IMAGE_LAYOUT_GENERAL,
                               VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mAccumulationPathTracer->execute(cmd, accDesc, renderWidth, renderHeight, frameIndex);
            recordImageBarrier(cmd, accOut, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            finalPathTracerImage = accOut;
        }
    }

    {
        // Tonemap
        {
            recordImageBarrier(cmd, mView[imageIndex]->textureTonemapImage, VK_IMAGE_LAYOUT_GENERAL,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            // recordBarrier(cmd, mResManager->getVkImage(tmpImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            //             VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            TonemapDesc toneDesc{};
            Tonemapparam& toneParams = toneDesc.constants;
            toneParams.dimension.x = renderWidth;
            toneParams.dimension.y = renderHeight;
            toneDesc.input = finalPathTracerImage;
            toneDesc.output = mView[imageIndex]->textureTonemapImage;
            mTonemap->execute(cmd, toneDesc, renderWidth, renderHeight, frameIndex);
            finalImage = mView[imageIndex]->textureTonemapImage;
        }

        // Upscale
        if (1)
        {
            recordImageBarrier(cmd, finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            
            UpscaleDesc upscaleDesc{};
            Upscalepassparam& upscalePassParam = upscaleDesc.constants;
            upscalePassParam.dimension.x = finalWidth;
            upscalePassParam.dimension.y = finalHeight;
            upscalePassParam.invDimension.x = 1.0f / (float)finalWidth;
            upscalePassParam.invDimension.y = 1.0f / (float)finalHeight;
            
            upscaleDesc.input = finalImage;
            upscaleDesc.output = mView[imageIndex]->textureUpscaleImage;

            mUpscalePass->execute(cmd, upscaleDesc, finalWidth, finalHeight, frameIndex);

            recordImageBarrier(cmd, finalImage, VK_IMAGE_LAYOUT_GENERAL,
                               VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            finalImage = mView[imageIndex]->textureUpscaleImage;
        }
    }

    // Copy to swapchain image
    recordImageBarrier(cmd, finalImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkDeviceSize imageSize = finalWidth * finalHeight * 16;
    Buffer* stagingBuffer = resManager->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = finalWidth;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.mipLevel = 0;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = finalWidth;
    region.imageExtent.height = finalHeight;
    region.imageExtent.depth = 1;
    vkCmdCopyImageToBuffer(cmd, resManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, resManager->getVkBuffer(stagingBuffer), 1, &region);

    //    VkOffset3D blitSize{};
    //    blitSize.x = finalWidth;
    //    blitSize.y = finalHeight;
    //    blitSize.z = 1;
    //    VkImageBlit imageBlitRegion{};
    //    imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageBlitRegion.srcSubresource.layerCount = 1;
    //    imageBlitRegion.srcOffsets[1] = blitSize;
    //    imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageBlitRegion.dstSubresource.layerCount = 1;
    //    imageBlitRegion.dstOffsets[1] = blitSize;
    //    vkCmdBlitImage(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);

    //    recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
    //                  VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    //    recordBarrier(cmd, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    //}

    // copy current depth from gbuffer to prev gbuffer
    //{
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //                  VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                  VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //    VkImageCopy region{};
    //    region.extent.width = renderWidth;
    //    region.extent.height = renderHeight;
    //    region.extent.depth = 1;

    //    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //    region.srcSubresource.mipLevel = 0;
    //    region.srcSubresource.layerCount = 1;
    //    region.srcSubresource.baseArrayLayer = 0;

    //    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //    region.dstSubresource.mipLevel = 0;
    //    region.dstSubresource.layerCount = 1;
    //    region.dstSubresource.baseArrayLayer = 0;

    //    vkCmdCopyImage(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //}

    // assign prev resources for next frame rendering
    mPrevView = mView[imageIndex];

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkResetFences(mDevice, 1, &currFrame.inFlightFence);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, currFrame.inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vkWaitForFences(mDevice, 1, &currFrame.inFlightFence, true, UINT64_MAX);

    vkDeviceWaitIdle(mDevice);

    void* data = resManager->getMappedMemory(stagingBuffer);
    memcpy((void*)outPixels, data, static_cast<size_t>(imageSize));

    ++mFrameNumber;
}

#include "RenderPass.h"

#include "Camera.h"
#include "Instancer.h"
#include "Material.h"
#include "Mesh.h"
#include "RenderBuffer.h"
#include "Tokens.h"

#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/hd/rprim.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_decompose.hpp>

static const char* DEFAULT_MTLX_DOC =
    "<?xml version=\"1.0\"?>"
    "<materialx version=\"1.38\" colorspace=\"lin_rec709\">"
    "  <UsdPreviewSurface name=\"SR_Invalid\" type=\"surfaceshader\">"
    "    <input name=\"diffuseColor\" type=\"color3\" value=\"1.0, 0.0, 1.0\" />"
    "    <input name=\"roughness\" type=\"float\" value=\"1.0\" />"
    "  </UsdPreviewSurface>"
    "  <surfacematerial name=\"USD_Invalid\" type=\"material\">"
    "    <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"SR_Invalid\" />"
    "  </surfacematerial>"
    "</materialx>";

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKRenderPass::HdNeVKRenderPass(HdRenderIndex* index,
                                         const HdRprimCollection& collection,
                                         const HdRenderSettingsMap& settings)
    : HdRenderPass(index, collection), m_settings(settings), m_isConverged(false), m_lastSceneStateVersion(UINT32_MAX), m_lastRenderSettingsVersion(UINT32_MAX)
{
}

HdNeVKRenderPass::~HdNeVKRenderPass()
{
}

bool HdNeVKRenderPass::IsConverged() const
{
    return m_isConverged;
}


//  valid range of coordinates [-1; 1]
uint32_t packNormal(const glm::float3& normal)
{
    uint32_t packed = (uint32_t)((normal.x + 1.0f) / 2.0f * 511.99999f);
    packed += (uint32_t)((normal.y + 1.0f) / 2.0f * 511.99999f) << 10;
    packed += (uint32_t)((normal.z + 1.0f) / 2.0f * 511.99999f) << 20;
    return packed;
}

void HdNeVKRenderPass::_BakeMeshInstance(const HdNeVKMesh* mesh,
                                            GfMatrix4d transform,
                                            uint32_t materialIndex)
{
    GfMatrix4d normalMatrix = transform.GetInverse().GetTranspose();

    const std::vector<GfVec3f>& meshPoints = mesh->GetPoints();
    const std::vector<GfVec3f>& meshNormals = mesh->GetNormals();
    const std::vector<GfVec3i>& meshFaces = mesh->GetFaces();
    TF_VERIFY(meshPoints.size() == meshNormals.size());
    const size_t vertexCount = meshPoints.size();

    std::vector<nevk::Scene::Vertex> vertices(vertexCount);
    std::vector<uint32_t> indices(meshFaces.size() * 3);

    for (size_t j = 0; j < meshFaces.size(); ++j)
    {
        const GfVec3i& vertexIndices = meshFaces[j];
        indices[j * 3 + 0] = vertexIndices[0];
        indices[j * 3 + 1] = vertexIndices[1];
        indices[j * 3 + 2] = vertexIndices[2];
    }
    glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
    for (size_t j = 0; j < vertexCount; ++j)
    {
        const GfVec3f& point = meshPoints[j];
        const GfVec3f& normal = meshNormals[j];

        nevk::Scene::Vertex& vertex = vertices[j];
        vertex.pos[0] = point[0];
        vertex.pos[1] = point[1];
        vertex.pos[2] = point[2];

        glm::float3 glmNormal = glm::float3(normal[0], normal[1], normal[2]);
        vertex.normal = packNormal(glmNormal);
        sum += vertex.pos;
    }
    const glm::float3 massCenter = sum / (float)vertexCount;
    int matId = 0;

    glm::float4x4 glmTransform;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            glmTransform[i][j] = (float)transform[i][j];
        }
    }

    uint32_t meshId = mScene.createMesh(vertices, indices);
    assert(meshId != -1);
    uint32_t instId = mScene.createInstance(meshId, matId, glmTransform, massCenter);
    assert(instId != -1);
}

void HdNeVKRenderPass::_BakeMeshes(HdRenderIndex* renderIndex,
                                   GfMatrix4d rootTransform)
{
    TfHashMap<SdfPath, uint32_t, SdfPath::Hash> materialMapping;
    materialMapping[SdfPath::EmptyPath()] = 0;
    
    Material defaultMaterial{};

    defaultMaterial.baseColorFactor = glm::float4(1.0f);
    defaultMaterial.diffuse = defaultMaterial.baseColorFactor;
    defaultMaterial.illum = 2; // opaque
    defaultMaterial.texBaseColor = -1; 
    defaultMaterial.texNormalId = -1;
    defaultMaterial.texEmissive = -1;

   // mScene.addMaterial(defaultMaterial);

    //materials.push_back(m_defaultMaterial);

    for (const auto& rprimId : renderIndex->GetRprimIds())
    {
        const HdRprim* rprim = renderIndex->GetRprim(rprimId);

        if (!dynamic_cast<const HdMesh*>(rprim))
        {
            continue;
        }

        const HdNeVKMesh* mesh = dynamic_cast<const HdNeVKMesh*>(rprim);

        VtMatrix4dArray transforms;
        const SdfPath& instancerId = mesh->GetInstancerId();

        if (instancerId.IsEmpty())
        {
            transforms.resize(1);
            transforms[0] = GfMatrix4d(1.0);
        }
        else
        {
            HdInstancer* boxedInstancer = renderIndex->GetInstancer(instancerId);
            HdNeVKInstancer* instancer = dynamic_cast<HdNeVKInstancer*>(boxedInstancer);

            const SdfPath& meshId = mesh->GetId();
            transforms = instancer->ComputeInstanceTransforms(meshId);
        }

        const SdfPath& materialId = mesh->GetMaterialId();
        uint32_t materialIndex = 0;
        std::cout << "runner" << std::endl;
        if (materialMapping.find(materialId) != materialMapping.end())
        {
            materialIndex = materialMapping[materialId];
        }
        else
        {
            HdSprim* sprim = renderIndex->GetSprim(HdPrimTypeTokens->material, materialId);
            HdNeVKMaterial* material = dynamic_cast<HdNeVKMaterial*>(sprim);

            if (material)
            {
                std::string code = material->GetNeVKMaterial();
                std::cout << code << std::endl;
                nevk::Scene::MaterialX material;
                material.code = code;
                materialIndex = mScene.materialsCode.size();
                mScene.materialsCode.push_back(material);
                mScene.addMaterial(defaultMaterial);
            }
        }

        const GfMatrix4d& prototypeTransform = mesh->GetPrototypeTransform();

        for (size_t i = 0; i < transforms.size(); i++)
        {
            GfMatrix4d transform = prototypeTransform * transforms[i]; // *rootTransform;
            //GfMatrix4d transform = GfMatrix4d(1.0);
            _BakeMeshInstance(mesh, transform, materialIndex);
        }
    }
    printf("Meshes: %zu\n", mScene.getMeshes().size());
    printf("Instances: %zu\n", mScene.getInstances().size());
    printf("Materials: %zu\n", mScene.getMaterials().size());
    fflush(stdout);
}

void HdNeVKRenderPass::_ConstructNeVKCamera(const HdNeVKCamera& camera)
{
    // We transform the scene into camera space at the beginning, so for
    // subsequent camera transforms, we need to 'substract' the initial transform.
    GfMatrix4d absInvViewMatrix = camera.GetTransform();
    GfMatrix4d relViewMatrix = absInvViewMatrix; //*m_rootMatrix;

    GfVec3d position = relViewMatrix.Transform(GfVec3d(0.0, 0.0, 0.0));
    GfVec3d forward = relViewMatrix.TransformDir(GfVec3d(0.0, 0.0, -1.0));
    GfVec3d up = relViewMatrix.TransformDir(GfVec3d(0.0, 1.0, 0.0));

    forward.Normalize();
    up.Normalize();

    bool isRH = relViewMatrix.IsRightHanded();

    glm::float4x4 xform;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            xform[i][j] = (float)relViewMatrix[i][j];
        }
    }

    //GfMatrix4d perspMatrix = camera.ComputeProjectionMatrix();
    GfMatrix4d perspMatrix = camera.GetProjectionMatrix();
    glm::float4x4 persp;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            persp[i][j] = (float)perspMatrix[i][j];
        }
    }

    nevk::Camera nevkCamera;

    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(xform, scale, rotation, translation, skew, perspective);
        rotation = glm::conjugate(rotation);
        nevkCamera.position = translation * scale;
        nevkCamera.mOrientation = rotation;
    }
    
    nevkCamera.matrices.perspective = persp;
    nevkCamera.matrices.invPerspective = glm::inverse(persp);

    //GfQuatd orient = relViewMatrix.ExtractRotationQuat().GetConjugate();
    //const double* im = orient.GetImaginary().data();
    //nevkCamera.mOrientation = glm::quat((float)orient.GetReal(), (float)im[0], (float)im[1], (float)im[2]);

    //nevkCamera.position = glm::float3(0.0f, 0.0f, 15.0f);

    //TODO: debug:
    //nevkCamera.setPerspective(45.0f, (float)800 / (float)600, 0.1f, 10000.0f);
    //nevkCamera.fov = glm::degrees(camera.GetVFov());
    //nevkCamera.setRotation(glm::quat({ 1.0f, 0.0f, 0.0f, 0.0f }));

    mScene.addCamera(nevkCamera);
}

void HdNeVKRenderPass::_Execute(const HdRenderPassStateSharedPtr& renderPassState,
                                const TfTokenVector& renderTags)
{
    TF_UNUSED(renderTags);

    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    m_isConverged = false;

    const auto* camera = dynamic_cast<const HdNeVKCamera*>(renderPassState->GetCamera());

    if (!camera)
    {
        return;
    }

    const HdRenderPassAovBindingVector& aovBindings = renderPassState->GetAovBindings();

    if (aovBindings.empty())
    {
        return;
    }

    const HdRenderPassAovBinding* colorAovBinding = nullptr;

    for (const HdRenderPassAovBinding& aovBinding : aovBindings)
    {
        if (aovBinding.aovName != HdAovTokens->color)
        {
            HdNeVKRenderBuffer* renderBuffer = dynamic_cast<HdNeVKRenderBuffer*>(aovBinding.renderBuffer);
            renderBuffer->SetConverged(true);
            continue;
        }

        colorAovBinding = &aovBinding;
    }

    if (!colorAovBinding)
    {
        return;
    }

    HdRenderIndex* renderIndex = GetRenderIndex();
    HdChangeTracker& changeTracker = renderIndex->GetChangeTracker();
    HdRenderDelegate* renderDelegate = renderIndex->GetRenderDelegate();
    HdNeVKRenderBuffer* renderBuffer = dynamic_cast<HdNeVKRenderBuffer*>(colorAovBinding->renderBuffer);

    uint32_t sceneStateVersion = changeTracker.GetSceneStateVersion();
    uint32_t renderSettingsStateVersion = renderDelegate->GetRenderSettingsVersion();
    bool sceneChanged = (sceneStateVersion != m_lastSceneStateVersion);
    bool renderSettingsChanged = (renderSettingsStateVersion != m_lastRenderSettingsVersion);

    if (!sceneChanged && !renderSettingsChanged)
    {
        renderBuffer->SetConverged(true);
        return;
    }

    renderBuffer->SetConverged(false);

    m_lastSceneStateVersion = sceneStateVersion;
    m_lastRenderSettingsVersion = renderSettingsStateVersion;


    // Transform scene into camera space to increase floating point precision.
    GfMatrix4d viewMatrix = camera->GetTransform().GetInverse();

    _BakeMeshes(renderIndex, viewMatrix);

    m_rootMatrix = viewMatrix;

    _ConstructNeVKCamera(*camera);

    mRender = new nevk::PtRender();
    mRender->setScene(&mScene);
    mRender->init();

    nevk::Scene::RectLightDesc desc{};
    desc.color = glm::float3(1.0f);
    desc.height = 0.4f;
    desc.width = 0.4f;
    desc.position = glm::float3(0, 1.1, 0.67);
    desc.orientation = glm::float3(179.68, 29.77, -89.97);
    desc.intensity = 160.0f;

    mScene.createLight(desc);

    float* img_data = (float*)renderBuffer->Map();

    mRender->drawFrame((uint8_t*)img_data);

    renderBuffer->Unmap();
    renderBuffer->SetConverged(true);

    m_isConverged = true;
}

PXR_NAMESPACE_CLOSE_SCOPE

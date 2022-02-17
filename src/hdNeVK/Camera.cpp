#include "Camera.h"

#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/camera.h>

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_decompose.hpp>

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKCamera::HdNeVKCamera(const SdfPath& id, nevk::Scene& scene)
    : HdCamera(id), mScene(scene), m_vfov(M_PI_2)
{
    const std::string& name = id.GetString();
    nevk::Camera nevkCamera;
    nevkCamera.name = name;
    mCameraIndex = mScene.addCamera(nevkCamera);
}

HdNeVKCamera::~HdNeVKCamera()
{
}

float HdNeVKCamera::GetVFov() const
{
    return m_vfov;
}

uint32_t HdNeVKCamera::GetCameraIndex() const
{
    return mCameraIndex;
}

void HdNeVKCamera::Sync(HdSceneDelegate* sceneDelegate,
                        HdRenderParam* renderParam,
                        HdDirtyBits* dirtyBits)
{
    HdDirtyBits dirtyBitsCopy = *dirtyBits;

    HdCamera::Sync(sceneDelegate, renderParam, &dirtyBitsCopy);
    if (*dirtyBits & DirtyBits::DirtyParams)
    {
        // See https://wiki.panotools.org/Field_of_View
        float aperture = _verticalAperture * GfCamera::APERTURE_UNIT;
        float focalLength = _focalLength * GfCamera::FOCAL_LENGTH_UNIT;
        float vfov = 2.0f * std::atanf(aperture / (2.0f * focalLength));

        m_vfov = vfov;
        
        mScene.updateCamera(_ConstructNeVKCamera(), mCameraIndex);
    }

    *dirtyBits = DirtyBits::Clean;
}

HdDirtyBits HdNeVKCamera::GetInitialDirtyBitsMask() const
{
    return DirtyBits::DirtyParams |
           DirtyBits::DirtyTransform;
}

nevk::Camera HdNeVKCamera::_ConstructNeVKCamera()
{
    nevk::Camera nevkCamera;

#ifdef __APPLE__
    GfMatrix4d perspMatrix = ComputeProjectionMatrix();
#else
    GfMatrix4d perspMatrix = GetProjectionMatrix();
#endif // __APPLE__
    GfMatrix4d absInvViewMatrix = GetTransform();
    GfMatrix4d relViewMatrix = absInvViewMatrix; //*m_rootMatrix;
    glm::float4x4 xform;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            xform[i][j] = (float)relViewMatrix[i][j];
        }
    }
    glm::float4x4 persp;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            persp[i][j] = (float)perspMatrix[i][j];
        }
    }
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
    nevkCamera.fov = glm::degrees(GetVFov());

    const std::string& name = GetId().GetString();
    nevkCamera.name = name;

    return nevkCamera;
}

PXR_NAMESPACE_CLOSE_SCOPE

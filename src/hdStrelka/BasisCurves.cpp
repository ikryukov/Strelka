#include "BasisCurves.h"

PXR_NAMESPACE_OPEN_SCOPE
void HdStrelkaBasisCurves::Sync(HdSceneDelegate* sceneDelegate,
                                HdRenderParam* renderParam,
                                HdDirtyBits* dirtyBits,
                                const TfToken& reprToken)
{
    TF_UNUSED(renderParam);
    TF_UNUSED(reprToken);

    HdRenderIndex& renderIndex = sceneDelegate->GetRenderIndex();

    const SdfPath& id = GetId();
    const char* curveName = id.GetText();
    mName = curveName;
    printf("Curve Name: %s\n", curveName);

    _UpdateGeometry(sceneDelegate);

    bool updateGeometry = (*dirtyBits & HdChangeTracker::DirtyPoints) | (*dirtyBits & HdChangeTracker::DirtyNormals) |
                          (*dirtyBits & HdChangeTracker::DirtyTopology);

    *dirtyBits = HdChangeTracker::Clean;

    if (!updateGeometry)
    {
       return;
    }

    m_faces.clear();
    mPoints.clear();
    mNormals.clear();


}

bool HdStrelkaBasisCurves::_FindPrimvar(HdSceneDelegate* sceneDelegate, TfToken primvarName, HdInterpolation& interpolation) const
{
    HdInterpolation interpolations[] = {
        HdInterpolation::HdInterpolationVertex,   HdInterpolation::HdInterpolationFaceVarying,
        HdInterpolation::HdInterpolationConstant, HdInterpolation::HdInterpolationUniform,
        HdInterpolation::HdInterpolationVarying,  HdInterpolation::HdInterpolationInstance
    };
    for (HdInterpolation i : interpolations)
    {
        const auto& primvarDescs = GetPrimvarDescriptors(sceneDelegate, i);
        for (const HdPrimvarDescriptor& primvar : primvarDescs)
        {
            if (primvar.name == primvarName)
            {
                interpolation = i;
                return true;
            }
        }
    }
    return false;
}

void HdStrelkaBasisCurves::_PullPrimvars(HdSceneDelegate* sceneDelegate,
                                  VtVec3fArray& points,
                                  VtVec3fArray& normals,
                                  VtFloatArray& widths,
                                  bool& indexedNormals,
                                  bool& indexedUVs,
                                  GfVec3f& color,
                                  bool& hasColor) const
{
    const SdfPath& id = GetId();
    // Handle points.
    HdInterpolation pointInterpolation;
    bool foundPoints = _FindPrimvar(sceneDelegate, HdTokens->points, pointInterpolation);

    if (!foundPoints)
    {
        TF_RUNTIME_ERROR("Points primvar not found!");
        return;
    }
    else if (pointInterpolation != HdInterpolation::HdInterpolationVertex)
    {
        TF_RUNTIME_ERROR("Points primvar is not vertex-interpolated!");
        return;
    }

    VtValue boxedPoints = sceneDelegate->Get(id, HdTokens->points);
    points = boxedPoints.Get<VtVec3fArray>();

    // Handle color.
    HdInterpolation colorInterpolation;
    bool foundColor = _FindPrimvar(sceneDelegate, HdTokens->displayColor, colorInterpolation);

    if (foundColor && colorInterpolation == HdInterpolation::HdInterpolationConstant)
    {
        VtValue boxedColors = sceneDelegate->Get(id, HdTokens->displayColor);
        const VtVec3fArray& colors = boxedColors.Get<VtVec3fArray>();
        color = colors[0];
        hasColor = true;
    }

    HdBasisCurvesTopology topology = GetBasisCurvesTopology(sceneDelegate);

    // Handle normals.
    HdInterpolation normalInterpolation;
    bool foundNormals = _FindPrimvar(sceneDelegate, HdTokens->normals, normalInterpolation);

    if (foundNormals && normalInterpolation == HdInterpolation::HdInterpolationVarying)
    {
        VtValue boxedNormals = sceneDelegate->Get(id, HdTokens->normals);
        normals = boxedNormals.Get<VtVec3fArray>();
        indexedNormals = true;
    }

    // Handle width.
    HdInterpolation widthInterpolation;
    bool foundWidth = _FindPrimvar(sceneDelegate, HdTokens->widths, widthInterpolation);

    if (foundWidth)
    {
        VtValue boxedWidths = sceneDelegate->Get(id, HdTokens->widths);
        widths = boxedWidths.Get<VtFloatArray>();
    }
}

void HdStrelkaBasisCurves::_UpdateGeometry(HdSceneDelegate* sceneDelegate)
{
    const HdBasisCurvesTopology& topology = GetBasisCurvesTopology(sceneDelegate);
    const SdfPath& id = GetId();

    VtIntArray vertexCount = topology.GetCurveVertexCounts();

    VtVec3fArray points;
    VtVec3fArray normals;
    VtFloatArray widths;
    bool indexedNormals;
    bool indexedUVs;
    bool hasColor = true;
    _PullPrimvars(sceneDelegate, points, normals, widths, indexedNormals, indexedUVs, mColor, hasColor);

    for (int i = 0; i < points.size(); i++)
    {

        printf("Points: %f %f %f \n", points[i][0], points[i][1], points[i][2]);

    }

    for (int i = 0; i < normals.size(); i++)
    {
        printf("Normals: %f %f %f \n", normals[i][0], normals[i][1], normals[i][2]);
    }

    for (int i = 0; i < widths.size(); i++)
    {
        printf("Widths: %f\n", widths[i]);
    }

}

HdStrelkaBasisCurves::HdStrelkaBasisCurves(const SdfPath& id, oka::Scene* scene)
    : HdBasisCurves(id),  mScene(scene)
{
}

HdStrelkaBasisCurves::~HdStrelkaBasisCurves()
{
}

HdDirtyBits HdStrelkaBasisCurves::GetInitialDirtyBitsMask() const
{
    return HdChangeTracker::DirtyPoints | HdChangeTracker::DirtyNormals | HdChangeTracker::DirtyTopology |
           HdChangeTracker::DirtyInstancer | HdChangeTracker::DirtyInstanceIndex | HdChangeTracker::DirtyTransform |
           HdChangeTracker::DirtyMaterialId | HdChangeTracker::DirtyPrimvar;
}

HdDirtyBits HdStrelkaBasisCurves::_PropagateDirtyBits(HdDirtyBits bits) const
{
    return bits;
}

void HdStrelkaBasisCurves::_InitRepr(const TfToken& reprName, HdDirtyBits* dirtyBits)
{
    TF_UNUSED(reprName);
    TF_UNUSED(dirtyBits);
}

PXR_NAMESPACE_CLOSE_SCOPE

#include "Mesh.h"

#include <pxr/imaging/hd/instancer.h>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/smoothNormals.h>
#include <pxr/imaging/hd/vertexAdjacency.h>

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKMesh::HdNeVKMesh(const SdfPath& id)
    : HdMesh(id), m_prototypeTransform(1.0), m_color(0.0, 0.0, 0.0), m_hasColor(false)
{
}

HdNeVKMesh::~HdNeVKMesh()
{
}

void HdNeVKMesh::Sync(HdSceneDelegate* sceneDelegate,
                      HdRenderParam* renderParam,
                      HdDirtyBits* dirtyBits,
                      const TfToken& reprToken)
{
    TF_UNUSED(renderParam);
    TF_UNUSED(reprToken);

    HdRenderIndex& renderIndex = sceneDelegate->GetRenderIndex();

    if ((*dirtyBits & HdChangeTracker::DirtyInstancer) |
        (*dirtyBits & HdChangeTracker::DirtyInstanceIndex))
    {
        HdDirtyBits dirtyBitsCopy = *dirtyBits;

        _UpdateInstancer(sceneDelegate, &dirtyBitsCopy);

        const SdfPath& instancerId = GetInstancerId();

        HdInstancer::_SyncInstancerAndParents(renderIndex, instancerId);
    }

    const SdfPath& id = GetId();

    if (*dirtyBits & HdChangeTracker::DirtyMaterialId)
    {
        const SdfPath& materialId = sceneDelegate->GetMaterialId(id);

        SetMaterialId(materialId);
    }

    if (*dirtyBits & HdChangeTracker::DirtyTransform)
    {
        m_prototypeTransform = sceneDelegate->GetTransform(id);
    }

    bool updateGeometry =
        (*dirtyBits & HdChangeTracker::DirtyPoints) |
        (*dirtyBits & HdChangeTracker::DirtyNormals) |
        (*dirtyBits & HdChangeTracker::DirtyTopology);

    *dirtyBits = HdChangeTracker::Clean;

    if (!updateGeometry)
    {
        return;
    }

    m_faces.clear();
    m_points.clear();
    m_normals.clear();

    _UpdateGeometry(sceneDelegate);
}

void HdNeVKMesh::_UpdateGeometry(HdSceneDelegate* sceneDelegate)
{
    const HdMeshTopology& topology = GetMeshTopology(sceneDelegate);
    const SdfPath& id = GetId();
    HdMeshUtil meshUtil(&topology, id);

    VtVec3iArray indices;
    VtIntArray primitiveParams;
    meshUtil.ComputeTriangleIndices(&indices, &primitiveParams);

    VtVec3fArray points;
    VtVec3fArray normals;
    bool indexedNormals;
    _PullPrimvars(sceneDelegate, points, normals, indexedNormals, m_color, m_hasColor);

    for (int i = 0; i < indices.size(); i++)
    {
        GfVec3i newFaceIndices(i * 3 + 0, i * 3 + 1, i * 3 + 2);
        m_faces.push_back(newFaceIndices);

        const GfVec3i& faceIndices = indices[i];
        m_points.push_back(points[faceIndices[0]]);
        m_points.push_back(points[faceIndices[1]]);
        m_points.push_back(points[faceIndices[2]]);
        m_normals.push_back(normals[indexedNormals ? faceIndices[0] : newFaceIndices[0]]);
        m_normals.push_back(normals[indexedNormals ? faceIndices[1] : newFaceIndices[1]]);
        m_normals.push_back(normals[indexedNormals ? faceIndices[2] : newFaceIndices[2]]);
    }
}

bool HdNeVKMesh::_FindPrimvar(HdSceneDelegate* sceneDelegate,
                              TfToken primvarName,
                              HdInterpolation& interpolation) const
{
    HdInterpolation interpolations[] = {
        HdInterpolation::HdInterpolationVertex,
        HdInterpolation::HdInterpolationFaceVarying,
        HdInterpolation::HdInterpolationConstant,
        HdInterpolation::HdInterpolationUniform,
        HdInterpolation::HdInterpolationVarying,
        HdInterpolation::HdInterpolationInstance
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

void HdNeVKMesh::_PullPrimvars(HdSceneDelegate* sceneDelegate,
                               VtVec3fArray& points,
                               VtVec3fArray& normals,
                               bool& indexedNormals,
                               GfVec3f& color,
                               bool& hasColor) const
{
    const SdfPath& id = GetId();

    // Handle points.
    HdInterpolation pointInterpolation;
    bool foundPoints = _FindPrimvar(sceneDelegate,
                                    HdTokens->points,
                                    pointInterpolation);

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
    bool foundColor = _FindPrimvar(sceneDelegate,
                                   HdTokens->displayColor,
                                   colorInterpolation);

    if (foundColor && colorInterpolation == HdInterpolation::HdInterpolationConstant)
    {
        VtValue boxedColors = sceneDelegate->Get(id, HdTokens->displayColor);
        const VtVec3fArray& colors = boxedColors.Get<VtVec3fArray>();
        color = colors[0];
        hasColor = true;
    }

    // Handle normals.
    HdInterpolation normalInterpolation;
    bool foundNormals = _FindPrimvar(sceneDelegate,
                                     HdTokens->normals,
                                     normalInterpolation);

    if (foundNormals &&
        normalInterpolation == HdInterpolation::HdInterpolationVertex)
    {
        VtValue boxedNormals = sceneDelegate->Get(id, HdTokens->normals);
        normals = boxedNormals.Get<VtVec3fArray>();
        indexedNormals = true;
        return;
    }

    HdMeshTopology topology = GetMeshTopology(sceneDelegate);

    if (foundNormals &&
        normalInterpolation == HdInterpolation::HdInterpolationFaceVarying)
    {
        VtValue boxedFvNormals = sceneDelegate->Get(id, HdTokens->normals);
        const VtVec3fArray& fvNormals = boxedFvNormals.Get<VtVec3fArray>();

        HdMeshUtil meshUtil(&topology, id);
        VtValue boxedTriangulatedNormals;
        if (!meshUtil.ComputeTriangulatedFaceVaryingPrimvar(
                fvNormals.cdata(),
                fvNormals.size(),
                HdTypeFloatVec3,
                &boxedTriangulatedNormals))
        {
            TF_CODING_ERROR("Unable to triangulate face-varying normals of %s", id.GetText());
        }

        normals = boxedTriangulatedNormals.Get<VtVec3fArray>();
        indexedNormals = false;
        return;
    }

    Hd_VertexAdjacency adjacency;
    adjacency.BuildAdjacencyTable(&topology);
    normals = Hd_SmoothNormals::ComputeSmoothNormals(&adjacency, points.size(), points.cdata());
    indexedNormals = true;
}

const TfTokenVector BUILTIN_PRIMVAR_NAMES = {
    HdTokens->points,
    HdTokens->normals
};

const TfTokenVector& HdNeVKMesh::GetBuiltinPrimvarNames() const
{
    return BUILTIN_PRIMVAR_NAMES;
}

const std::vector<GfVec3f>& HdNeVKMesh::GetPoints() const
{
    return m_points;
}

const std::vector<GfVec3f>& HdNeVKMesh::GetNormals() const
{
    return m_normals;
}

const std::vector<GfVec3i>& HdNeVKMesh::GetFaces() const
{
    return m_faces;
}

const GfMatrix4d& HdNeVKMesh::GetPrototypeTransform() const
{
    return m_prototypeTransform;
}

const GfVec3f& HdNeVKMesh::GetColor() const
{
    return m_color;
}

bool HdNeVKMesh::HasColor() const
{
    return m_hasColor;
}

HdDirtyBits HdNeVKMesh::GetInitialDirtyBitsMask() const
{
    return HdChangeTracker::DirtyPoints |
           HdChangeTracker::DirtyNormals |
           HdChangeTracker::DirtyTopology |
           HdChangeTracker::DirtyInstancer |
           HdChangeTracker::DirtyInstanceIndex |
           HdChangeTracker::DirtyTransform |
           HdChangeTracker::DirtyMaterialId;
}

HdDirtyBits HdNeVKMesh::_PropagateDirtyBits(HdDirtyBits bits) const
{
    return bits;
}

void HdNeVKMesh::_InitRepr(const TfToken& reprName,
                           HdDirtyBits* dirtyBits)
{
    TF_UNUSED(reprName);
    TF_UNUSED(dirtyBits);
}

PXR_NAMESPACE_CLOSE_SCOPE

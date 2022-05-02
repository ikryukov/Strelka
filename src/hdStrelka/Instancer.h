//    Copyright (C) 2021 Pablo Delgado Krämer
//
//        This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//                                         (at your option) any later version.
//
//                                         This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <pxr/imaging/hd/instancer.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdStrelkaInstancer final : public HdInstancer
{
public:
    HdStrelkaInstancer(HdSceneDelegate* delegate,
                       const SdfPath& id);

    ~HdStrelkaInstancer() override;

public:
    VtMatrix4dArray ComputeInstanceTransforms(const SdfPath& prototypeId);

    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

private:
    TfHashMap<TfToken, VtValue, TfToken::HashFunctor> m_primvarMap;
};

PXR_NAMESPACE_CLOSE_SCOPE

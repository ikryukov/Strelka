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

#include "mdlLogger.h"
#include "mdlRuntime.h"
#include "materials.h"

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <mi/mdl_sdk.h>

#include "texturemanager/texturemanager.h"

namespace oka
{
class MdlHlslCodeGen
{
public:
    explicit MdlHlslCodeGen(){};
    bool init(MdlRuntime& runtime);

    struct InternalMaterialInfo
    {
        mi::Size argument_block_index;
    };

    mi::base::Handle<const mi::neuraylib::ITarget_code> translate(const std::vector<const mi::neuraylib::ICompiled_material*>& materials,
                   std::string& hlslSrc, std::vector<InternalMaterialInfo>& internalsInfo);


private:
    bool appendMaterialToLinkUnit(uint32_t idx,
                                  const mi::neuraylib::ICompiled_material* compiledMaterial,
                                  mi::neuraylib::ILink_unit* linkUnit, mi::Size& argBlockIndex);

    std::unique_ptr<MdlNeurayLoader> mLoader;

    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::IMdl_backend> mBackend;
    mi::base::Handle<mi::neuraylib::IDatabase> mDatabase;
    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> mContext;
};
} // namespace oka

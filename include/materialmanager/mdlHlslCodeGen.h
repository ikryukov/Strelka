#pragma once
#include "mdlLogger.h"
#include "mdlRuntime.h"
#include "materials.h"

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <mi/mdl_sdk.h>

#include "texturemanager/texturemanager.h"

namespace nevk
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
} // namespace nevk

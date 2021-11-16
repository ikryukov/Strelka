#pragma once

#include <string>

#include <mi/base/handle.h>
#include <mi/neuraylib/icompiled_material.h>
#include <mi/neuraylib/idatabase.h>
#include <mi/neuraylib/itransaction.h>
#include <mi/neuraylib/imaterial_instance.h>
#include <mi/neuraylib/imdl_impexp_api.h>
#include <mi/neuraylib/imdl_execution_context.h>
#include <mi/neuraylib/imdl_factory.h>

#include "mdlRuntime.h"

namespace nevk
{
class MdlMaterialCompiler
{
public:
    MdlMaterialCompiler(MdlRuntime& runtime);

public:
    bool compileMaterial(const std::string& src,
                         const std::string& identifier,
                         mi::base::Handle<mi::neuraylib::ICompiled_material>& compiledMaterial);

private:
    bool createModule(mi::neuraylib::IMdl_execution_context* context,
                      const char* moduleName,
                      const char* mdlSrc);

    bool createCompiledMaterial(mi::neuraylib::IMdl_execution_context* context,
                                const char* moduleName,
                                const std::string& identifier,
                                mi::base::Handle<mi::neuraylib::ICompiled_material>& compiledMaterial);

private:
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::IDatabase> mDatabase;
    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<mi::neuraylib::IMdl_factory> mFactory;
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api> mImpExpApi;
};
}

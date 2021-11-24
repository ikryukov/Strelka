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
    bool createModule(const std::string& identifier,
                      std::string& moduleName);

    bool createMaterialInstace(const char* moduleName, const char* identifier, 
        mi::base::Handle<mi::neuraylib::IMaterial_instance>& matInstance);

    bool compileMaterial(mi::base::Handle<mi::neuraylib::IMaterial_instance>& instance,
                         mi::base::Handle<mi::neuraylib::ICompiled_material>& compiledMaterial);

    mi::base::Handle<mi::neuraylib::IMdl_factory>& getFactory();

private:
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::IDatabase> mDatabase;
    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<mi::neuraylib::IMdl_factory> mFactory;
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api> mImpExpApi;
};
}

#include "mdlMaterialCompiler.h"

#include <mi/mdl_sdk.h>

#include <atomic>
#include <cassert>
#include <iostream>

namespace oka
{
std::string _makeModuleName(const std::string& identifier)
{
    return "::" + identifier;
}

MdlMaterialCompiler::MdlMaterialCompiler(MdlRuntime& runtime)
{
    mLogger = mi::base::Handle<MdlLogger>(runtime.getLogger());
    mDatabase = mi::base::Handle<mi::neuraylib::IDatabase>(runtime.getDatabase());
    mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(runtime.getTransaction());
    mFactory = mi::base::Handle<mi::neuraylib::IMdl_factory>(runtime.getFactory());
    mImpExpApi = mi::base::Handle<mi::neuraylib::IMdl_impexp_api>(runtime.getImpExpApi());
}

bool MdlMaterialCompiler::createModule(const std::string& identifier,
                                       std::string& moduleName)
{
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(mFactory->create_execution_context());

    moduleName = _makeModuleName(identifier);

    mi::Sint32 result = mImpExpApi->load_module(mTransaction.get(), moduleName.c_str(), context.get());
    mLogger->flushContextMessages(context.get());
    return result == 0 || result == 1;
}

bool MdlMaterialCompiler::createMaterialInstace(const char* moduleName, const char* identifier, mi::base::Handle<mi::neuraylib::IMaterial_instance>& matInstance)
{
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(mFactory->create_execution_context());

    mi::base::Handle<const mi::IString> moduleDbName(mFactory->get_db_module_name(moduleName));
    mi::base::Handle<const mi::neuraylib::IModule> module(mTransaction->access<mi::neuraylib::IModule>(moduleDbName->get_c_str()));
    assert(module);

    std::string materialDbName = std::string(moduleDbName->get_c_str()) + "::" + identifier;
    mi::base::Handle<const mi::IArray> funcs(module->get_function_overloads(materialDbName.c_str(), (const mi::neuraylib::IExpression_list*)nullptr));
    if (funcs->get_length() == 0)
    {
        std::string errorMsg = std::string("Material with identifier ") + identifier + " not found in MDL module\n";
        mLogger->message(mi::base::MESSAGE_SEVERITY_ERROR, errorMsg.c_str());
        return false;
    }
    if (funcs->get_length() > 1)
    {
        std::string errorMsg = std::string("Ambigious material identifier ") + identifier + " for MDL module\n";
        mLogger->message(mi::base::MESSAGE_SEVERITY_ERROR, errorMsg.c_str());
        return false;
    }

    mi::base::Handle<const mi::IString> exactMaterialDbName(funcs->get_element<mi::IString>(0));
    // get material definition from database
    mi::base::Handle<const mi::neuraylib::IMaterial_definition> matDefinition(mTransaction->access<mi::neuraylib::IMaterial_definition>(exactMaterialDbName->get_c_str()));
    if (!matDefinition)
    {
        return false;
    }

    mi::Sint32 result;
    // instantiate material with default parameters and store in database
    matInstance = matDefinition->create_material_instance(NULL, &result);
    if (result != 0 || !matInstance)
    {
        return false;
    }
    return true;
}

bool MdlMaterialCompiler::compileMaterial(mi::base::Handle<mi::neuraylib::IMaterial_instance>& instance, mi::base::Handle<mi::neuraylib::ICompiled_material>& compiledMaterial)
{
    if (!instance)
    {
        // TODO: log error
        return false;
    }
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(mFactory->create_execution_context());
    auto flags = mi::neuraylib::IMaterial_instance::CLASS_COMPILATION;
    compiledMaterial = mi::base::Handle<mi::neuraylib::ICompiled_material>(instance->create_compiled_material(flags, context.get()));

    mLogger->flushContextMessages(context.get());
    return true;
}
mi::base::Handle<mi::neuraylib::IMdl_factory>& MdlMaterialCompiler::getFactory()
{
    return mFactory;
}
mi::base::Handle<mi::neuraylib::ITransaction>& MdlMaterialCompiler::getTransaction()
{
    return mTransaction;
}
} // namespace oka

#include "mdlMaterialCompiler.h"

#include <mi/mdl_sdk.h>

#include <atomic>
#include <cassert>

namespace nevk
{
const char* MODULE_PREFIX = "::nevk_";
std::atomic_uint32_t s_idCounter(0);

std::string _makeModuleName(const std::string& identifier)
{
    uint32_t uniqueId = ++s_idCounter;
    return "::" + identifier; //if mdl->hlsl
    //return std::string(MODULE_PREFIX) + std::to_string(uniqueId) + "_" + identifier;
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
                                       const char* mdlSrc, std::string& moduleName)
{
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(mFactory->create_execution_context());

    moduleName = _makeModuleName(identifier);

    mi::Sint32 result = mImpExpApi->load_module(mTransaction.get(), moduleName.c_str(), context.get()); // if mdl -> hlsl
    // mi::Sint32 result = m_impExpApi->load_module_from_string(m_transaction.get(), moduleName, mdlSrc, context); //if mtlx -> hlsl
    mLogger->flushContextMessages(context.get());
    return result == 0 || result == 1;
}

bool MdlMaterialCompiler::createCompiledMaterial(const char* moduleName,
                                                 const char* identifier,
                                                 mi::base::Handle<mi::neuraylib::ICompiled_material>& compiledMaterial)
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
    mi::base::Handle<const mi::neuraylib::IMaterial_definition> matDefinition(mTransaction->access<mi::neuraylib::IMaterial_definition>(exactMaterialDbName->get_c_str()));
    if (!matDefinition)
    {
        return false;
    }

    mi::Sint32 result;
    mi::base::Handle<mi::neuraylib::IMaterial_instance> matInstance(matDefinition->create_material_instance(NULL, &result));
    if (result != 0 || !matInstance)
    {
        return false;
    }

    auto flags = mi::neuraylib::IMaterial_instance::DEFAULT_OPTIONS; // Instance compilation, no class compilation.
    compiledMaterial = mi::base::Handle<mi::neuraylib::ICompiled_material>(matInstance->create_compiled_material(flags, context.get()));

    mLogger->flushContextMessages(context.get());
    return true;
}
}

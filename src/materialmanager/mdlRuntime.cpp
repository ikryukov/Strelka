#include "mdlRuntime.h"

#include <mi/mdl_sdk.h>

#include <vector>

namespace nevk
{
MdlRuntime::MdlRuntime()
{
}

MdlRuntime::~MdlRuntime()
{
    if (mTransaction)
    {
        mTransaction->commit();
    }
}

bool MdlRuntime::init(const char* paths[], uint32_t numPaths, const char* neurayPath, const char* imagePluginPath)
{
    mLoader = std::make_unique<MdlNeurayLoader>();
    if (!mLoader->init(neurayPath, imagePluginPath))
    {
        return false;
    }

    mi::base::Handle<mi::neuraylib::INeuray> neuray(mLoader->getNeuray());
    mi::base::Handle<mi::neuraylib::IMdl_configuration> config(neuray->get_api_component<mi::neuraylib::IMdl_configuration>());

    mLogger = mi::base::Handle<MdlLogger>(new MdlLogger());
    config->set_logger(mLogger.get());

    for (uint32_t i = 0; i < numPaths; i++)
    {
        if (config->add_mdl_path(paths[i]) != 0 || config->add_resource_path(paths[i]) != 0)
        {
            mLogger->message(mi::base::MESSAGE_SEVERITY_FATAL, "MDL file path not found, translation not possible");
            return false;
        }
    }

    mDatabase = mi::base::Handle<mi::neuraylib::IDatabase>(neuray->get_api_component<mi::neuraylib::IDatabase>());
    mi::base::Handle<mi::neuraylib::IScope> scope(mDatabase->get_global_scope());
    mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(scope->create_transaction());

    mFactory = mi::base::Handle<mi::neuraylib::IMdl_factory>(neuray->get_api_component<mi::neuraylib::IMdl_factory>());
    mImpExpApi = mi::base::Handle<mi::neuraylib::IMdl_impexp_api>(neuray->get_api_component<mi::neuraylib::IMdl_impexp_api>());
    mBackendApi = mi::base::Handle<mi::neuraylib::IMdl_backend_api>(neuray->get_api_component<mi::neuraylib::IMdl_backend_api>());
    return true;
}

mi::base::Handle<mi::neuraylib::INeuray> MdlRuntime::getNeuray()
{
    return mLoader->getNeuray();
}

mi::base::Handle<MdlLogger> MdlRuntime::getLogger()
{
    return mLogger;
}

mi::base::Handle<mi::neuraylib::IDatabase> MdlRuntime::getDatabase()
{
    return mDatabase;
}

mi::base::Handle<mi::neuraylib::ITransaction> MdlRuntime::getTransaction()
{
    return mTransaction;
}

mi::base::Handle<mi::neuraylib::IMdl_factory> MdlRuntime::getFactory()
{
    return mFactory;
}

mi::base::Handle<mi::neuraylib::IMdl_impexp_api> MdlRuntime::getImpExpApi()
{
    return mImpExpApi;
}

mi::base::Handle<mi::neuraylib::IMdl_backend_api> MdlRuntime::getBackendApi()
{
    return mBackendApi;
}
} // namespace nevk

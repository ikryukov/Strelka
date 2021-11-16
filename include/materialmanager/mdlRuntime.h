#pragma once

#include "mdlLogger.h"
#include "mdlNeurayLoader.h"

#include <mi/base/handle.h>
#include <mi/neuraylib/idatabase.h>
#include <mi/neuraylib/itransaction.h>
#include <mi/neuraylib/imdl_backend_api.h>
#include <mi/neuraylib/imdl_impexp_api.h>
#include <mi/neuraylib/imdl_factory.h>

#include <memory>
#include <vector>

namespace nevk
{
class MdlRuntime
{
public:
    MdlRuntime();
    ~MdlRuntime();

public:
    bool init(const char* resourcePath, const char* neurayPath,
              const std::vector<std::string>& mdlModulesPaths, const char* imagePluginPath);

    mi::base::Handle<MdlLogger> getLogger();
    mi::base::Handle<mi::neuraylib::IDatabase> getDatabase();
    mi::base::Handle<mi::neuraylib::ITransaction> getTransaction();
    mi::base::Handle<mi::neuraylib::IMdl_factory> getFactory();
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api> getImpExpApi();
    mi::base::Handle<mi::neuraylib::IMdl_backend_api> getBackendApi();
    mi::base::Handle<mi::neuraylib::INeuray> getNeuray();

    std::unique_ptr<MdlNeurayLoader> mLoader;
private:
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::IDatabase> mDatabase;
    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<mi::neuraylib::IMdl_factory> mFactory;
    mi::base::Handle<mi::neuraylib::IMdl_backend_api> mBackendApi;
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api> mImpExpApi;
};
}

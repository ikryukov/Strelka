#pragma once

#include "ShaderManager.h"

#include <mi/mdl_sdk.h>

namespace nevk
{

class MaterialManager
{
public:
    MaterialManager(){};
    ~MaterialManager(){};

    // load neurey
    bool initMDL(const char* resourcePath);
    void unloadDso();
    bool loadNeuray();
    bool loadDso(const char* resourcePath);

    [[nodiscard]] mi::base::Handle<mi::neuraylib::INeuray> getNeuray() const {
        return m_neuray;
    };
    void* m_dsoHandle;
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;

    // runtime material
    bool initMaterial(const char* mtlxmdlPath);
    mi::base::Handle<mi::neuraylib::IDatabase> m_database;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_factory;
    mi::base::Handle<mi::neuraylib::IMdl_backend_api> m_backendApi;
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api> m_impExpApi;

    // material compiler
    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    bool compileMaterial(const std::string& src, const std::string& identifier);
    bool createModule(mi::neuraylib::IMdl_execution_context* context, const char* moduleName, const char* mdlSrc);
    bool createCompiledMaterial(mi::neuraylib::IMdl_execution_context* context, const char* moduleName, const std::string& identifier);

    // hlsl code gen
    bool initCodeGen();
    bool translate(std::string& hlslSrc);
    bool appendMaterialToLinkUnit(uint32_t idx, const mi::neuraylib::ICompiled_material* compiledMaterial);
    mi::neuraylib::ILink_unit* linkedUnit;
    mi::base::Handle<mi::neuraylib::IMdl_backend> m_backend;
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> m_context;
};
} // namespace nevk

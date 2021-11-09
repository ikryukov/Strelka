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
    struct Material
    {

    };
    std::vector<Material> materials;
    uint32_t loadMaterial(const char* fileName);
    uint32_t destroyMaterial(uint32_t matId);

    bool initMDL(const char* resourcePath);
    void unloadDso();
    bool loadNeuray();
    bool loadDso(const char* resourcePath);

    mi::base::Handle<mi::neuraylib::INeuray> getNeuray() const;
    void* m_dsoHandle;
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
};
} // namespace nevk

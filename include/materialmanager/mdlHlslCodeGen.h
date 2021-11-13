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
    explicit MdlHlslCodeGen(nevk::TextureManager* texManager) : mTexManager(texManager){};
    bool init(MdlRuntime& runtime);

    mi::base::Handle<const mi::neuraylib::ITarget_code> translate(const std::vector<const mi::neuraylib::ICompiled_material*>& materials,
                   std::string& hlslSrc);

    bool loadTextures(mi::base::Handle<const mi::neuraylib::ITarget_code>& targetCode);

private:
    bool appendMaterialToLinkUnit(uint32_t idx,
                                  const mi::neuraylib::ICompiled_material* compiledMaterial,
                                  mi::neuraylib::ILink_unit* linkUnit);
    bool prepare_texture(
        const mi::base::Handle<mi::neuraylib::ITransaction>& transaction,
        const mi::base::Handle<mi::neuraylib::IImage_api>& image_api,
        const mi::base::Handle<const mi::neuraylib::ITarget_code>& code,
        mi::Size texture_index);

    nevk::TextureManager* mTexManager = nullptr;
    std::vector<Mdl_resource_info> info;
    std::unique_ptr<MdlNeurayLoader> m_loader;

    mi::base::Handle<MdlLogger> m_logger;
    mi::base::Handle<mi::neuraylib::IMdl_backend> m_backend;
    mi::base::Handle<mi::neuraylib::IDatabase> m_database;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> m_context;
};
} // namespace nevk

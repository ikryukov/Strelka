#include "mdlHlslCodeGen.h"

#include "materials.h"

#include <mi/mdl_sdk.h>

#include <cassert>
#include <sstream>

namespace nevk
{
const char* SCATTERING_FUNC_NAME = "mdl_bsdf_scattering";
const char* EMISSION_FUNC_NAME = "mdl_edf_emission";
const char* EMISSION_INTENSITY_FUNC_NAME = "mdl_edf_emission_intensity";
const char* MATERIAL_STATE_NAME = "Shading_state_material";

void _generateInitSwitch(std::stringstream& ss,
                         const char* funcName,
                         uint32_t caseCount)
{
    ss << "void " << funcName << "_init(in int idx, in " << MATERIAL_STATE_NAME << " sIn)\n";
    ss << "{\n";
    ss << "\tswitch(idx)\n";
    ss << "\t{\n";
    for (uint32_t i = 0; i < caseCount; i++)
    {
        ss << "\t\tcase " << i << ": " << funcName << "_" << i << "_init"
           << "(sIn); return;\n";
    }
    ss << "\t}\n";
    ss << "}\n";
}

void _generateEdfIntensitySwitch(std::stringstream& ss,
                                 uint32_t caseCount)
{
    ss << "float3 " << EMISSION_INTENSITY_FUNC_NAME << "(in int idx, in " << MATERIAL_STATE_NAME << " sIn)\n";
    ss << "{\n";
    ss << "\tswitch(idx)\n";
    ss << "\t{\n";
    for (uint32_t i = 0; i < caseCount; i++)
    {
        ss << "\t\tcase " << i << ": return " << EMISSION_INTENSITY_FUNC_NAME << "_" << i << "(sIn);\n";
    }
    ss << "\t}\n";
    ss << "\treturn float3(0.0, 0.0, 0.0);\n";
    ss << "}\n";
}

void _generateInOutSwitch(std::stringstream& ss,
                          const char* funcName,
                          const char* opName,
                          const char* inoutTypeName,
                          uint32_t caseCount)
{
    ss << "void " << funcName << "_" << opName << "(in int idx, inout " << inoutTypeName << " sInOut, in " << MATERIAL_STATE_NAME << " sIn)\n";
    ss << "{\n";
    ss << "\tswitch(idx)\n";
    ss << "\t{\n";
    for (uint32_t i = 0; i < caseCount; i++)
    {
        ss << "\t\tcase " << i << ": " << funcName << "_" << i << "_" << opName << "(sInOut, sIn); return;\n";
    }
    ss << "\t}\n";
    ss << "}\n";
}

bool MdlHlslCodeGen::init(MdlRuntime& runtime)
{
    mi::base::Handle<mi::neuraylib::IMdl_backend_api> backendApi(runtime.getBackendApi());
    m_backend = mi::base::Handle<mi::neuraylib::IMdl_backend>(backendApi->get_backend(mi::neuraylib::IMdl_backend_api::MB_HLSL));
    if (!m_backend.is_valid_interface())
    {
        m_logger->message(mi::base::MESSAGE_SEVERITY_FATAL, "HLSL backend not supported by MDL runtime");
        return false;
    }

    m_logger = mi::base::Handle<MdlLogger>(runtime.getLogger());

    m_loader = std::move(runtime.m_loader);
    mi::base::Handle<mi::neuraylib::IMdl_factory> factory(runtime.getFactory());
    m_context = mi::base::Handle<mi::neuraylib::IMdl_execution_context>(factory->create_execution_context());

    m_database = mi::base::Handle<mi::neuraylib::IDatabase>(runtime.getDatabase());
    m_transaction = mi::base::Handle<mi::neuraylib::ITransaction>(runtime.getTransaction());
    return true;
}

// Prepare the texture identified by the texture_index for use by the texture access functions
// on the GPU.
bool MdlHlslCodeGen::prepare_texture(
    const mi::base::Handle<mi::neuraylib::ITransaction>& transaction,
    const mi::base::Handle<mi::neuraylib::IImage_api>& image_api,
    const mi::base::Handle<const mi::neuraylib::ITarget_code>& code,
    mi::Size texture_index)
{
    // Get access to the texture data by the texture database name from the target code.
    mi::base::Handle<const mi::neuraylib::ITexture> texture(
        transaction->access<mi::neuraylib::ITexture>(code->get_texture(texture_index)));
    mi::base::Handle<const mi::neuraylib::IImage> image(
        transaction->access<mi::neuraylib::IImage>(texture->get_image()));
    mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas());
    mi::Uint32 tex_width = canvas->get_resolution_x();
    mi::Uint32 tex_height = canvas->get_resolution_y();
    mi::Uint32 tex_layers = canvas->get_layers_size();
    char const* image_type = image->get_type();

    if (canvas->get_tiles_size_x() != 1 || canvas->get_tiles_size_y() != 1)
    {
        m_logger->message(mi::base::MESSAGE_SEVERITY_ERROR, "The example does not support tiled images!");
        return false;
    }

    if (tex_layers != 1)
    {
        m_logger->message(mi::base::MESSAGE_SEVERITY_ERROR, "The example does not support layered images!");
        return false;
    }

    // For simplicity, the texture access functions are only implemented for float4 and gamma
    // is pre-applied here (all images are converted to linear space).
    // Convert to linear color space if necessary
    if (texture->get_effective_gamma() != 1.0f)
    {
        // Copy/convert to float4 canvas and adjust gamma from "effective gamma" to 1.
        mi::base::Handle<mi::neuraylib::ICanvas> gamma_canvas(
            image_api->convert(canvas.get(), "Color"));
        gamma_canvas->set_gamma(texture->get_effective_gamma());
        image_api->adjust_gamma(gamma_canvas.get(), 1.0f);
        canvas = gamma_canvas;
    }
    else if (strcmp(image_type, "Color") != 0 && strcmp(image_type, "Float32<4>") != 0)
    {
        // Convert to expected format
        canvas = image_api->convert(canvas.get(), "Color");
    }

    // This example supports only 2D textures
    mi::neuraylib::ITarget_code::Texture_shape texture_shape =
        code->get_texture_shape(texture_index);
    int resIndex = 0;
    if (texture_shape == mi::neuraylib::ITarget_code::Texture_shape_2d)
    {
        mi::base::Handle<const mi::neuraylib::ITile> tile(canvas->get_tile());
        mi::Float32 const* data = static_cast<mi::Float32 const*>(tile->get_data());
        uint id = mTexManager->loadTextureGltf(data, tex_width, tex_height, std::to_string(texture_index));
        info.push_back({id, 0, 0, 0});
    }

    return true;
}

bool MdlHlslCodeGen::loadTextures(mi::base::Handle<const mi::neuraylib::ITarget_code>& targetCode)
{
    // Acquire image API needed to prepare the textures
    mi::base::Handle<mi::neuraylib::INeuray> neuray(m_loader->getNeuray());
    mi::base::Handle<mi::neuraylib::IImage_api> image_api(neuray->get_api_component<mi::neuraylib::IImage_api>());

    if (targetCode->get_texture_count() > 0)
    {
        for (int i = 1; i < targetCode->get_texture_count(); ++i)
        {
            m_logger->message(mi::base::MESSAGE_SEVERITY_INFO, targetCode->get_texture_url(i));
            prepare_texture(m_transaction, image_api, targetCode, i);
        }
    }
    else
    {
        return false;
    }

    return true;
}

mi::base::Handle<const mi::neuraylib::ITarget_code> MdlHlslCodeGen::translate(const std::vector<const mi::neuraylib::ICompiled_material*>& materials,
                                                                              std::string& hlslSrc)
{
    mi::base::Handle<mi::neuraylib::ILink_unit> linkUnit(m_backend->create_link_unit(m_transaction.get(), m_context.get()));
    m_logger->flushContextMessages(m_context.get());

    if (!linkUnit)
    {
        throw "Failed to create link unit";
    }

    uint32_t materialCount = materials.size();
    for (uint32_t i = 0; i < materialCount; i++)
    {
        const mi::neuraylib::ICompiled_material* material = materials.at(i);
        assert(material);

        if (!appendMaterialToLinkUnit(i, material, linkUnit.get()))
        {
            throw "Failed to append material to the link unit";
        }
    }

    mi::base::Handle<const mi::neuraylib::ITarget_code> targetCode(m_backend->translate_link_unit(linkUnit.get(), m_context.get()));
    m_logger->flushContextMessages(m_context.get());

    if (!targetCode)
    {
        throw "No target code";
    }

    std::stringstream ss;
    ss << targetCode->get_code();

    _generateInOutSwitch(ss, SCATTERING_FUNC_NAME, "sample", "Bsdf_sample_data", materialCount);
    _generateInitSwitch(ss, SCATTERING_FUNC_NAME, materialCount);

    _generateInOutSwitch(ss, EMISSION_FUNC_NAME, "evaluate", "Edf_evaluate_data", materialCount);
    _generateInitSwitch(ss, EMISSION_FUNC_NAME, materialCount);

    _generateEdfIntensitySwitch(ss, materialCount);

    hlslSrc = ss.str();

    return targetCode;
}

bool MdlHlslCodeGen::appendMaterialToLinkUnit(uint32_t idx,
                                              const mi::neuraylib::ICompiled_material* compiledMaterial,
                                              mi::neuraylib::ILink_unit* linkUnit)
{
    std::string idxStr = std::to_string(idx);
    auto scatteringFuncName = std::string(SCATTERING_FUNC_NAME) + "_" + idxStr;
    auto emissionFuncName = std::string(EMISSION_FUNC_NAME) + "_" + idxStr;
    auto emissionIntensityFuncName = std::string(EMISSION_INTENSITY_FUNC_NAME) + "_" + idxStr;

    std::vector<mi::neuraylib::Target_function_description> genFunctions;
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.scattering", scatteringFuncName.c_str()));
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.emission.emission", emissionFuncName.c_str()));
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.emission.intensity", emissionIntensityFuncName.c_str()));

    mi::Sint32 result = linkUnit->add_material(
        compiledMaterial,
        genFunctions.data(),
        genFunctions.size(),
        m_context.get());

    m_logger->flushContextMessages(m_context.get());

    return result == 0;
}
} // namespace nevk

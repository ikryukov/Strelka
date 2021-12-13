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
    mBackend = mi::base::Handle<mi::neuraylib::IMdl_backend>(backendApi->get_backend(mi::neuraylib::IMdl_backend_api::MB_HLSL));
    if (!mBackend.is_valid_interface())
    {
        mLogger->message(mi::base::MESSAGE_SEVERITY_FATAL, "HLSL backend not supported by MDL runtime");
        return false;
    }

    mLogger = mi::base::Handle<MdlLogger>(runtime.getLogger());

    mLoader = std::move(runtime.mLoader);
    mi::base::Handle<mi::neuraylib::IMdl_factory> factory(runtime.getFactory());
    mContext = mi::base::Handle<mi::neuraylib::IMdl_execution_context>(factory->create_execution_context());

    mDatabase = mi::base::Handle<mi::neuraylib::IDatabase>(runtime.getDatabase());
    mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(runtime.getTransaction());
    return true;
}

mi::base::Handle<const mi::neuraylib::ITarget_code> MdlHlslCodeGen::translate(const std::vector<const mi::neuraylib::ICompiled_material*>& materials,
                                                                              std::string& hlslSrc,
                                                                              std::vector<InternalMaterialInfo>& internalsInfo)
{
    mi::base::Handle<mi::neuraylib::ILink_unit> linkUnit(mBackend->create_link_unit(mTransaction.get(), mContext.get()));
    mLogger->flushContextMessages(mContext.get());

    if (!linkUnit)
    {
        throw "Failed to create link unit";
    }

    uint32_t materialCount = materials.size();
    internalsInfo.resize(materialCount);
    mi::Size argBlockIndex;

    for (uint32_t i = 0; i < materialCount; i++)
    {
        const mi::neuraylib::ICompiled_material* material = materials.at(i);
        assert(material);

        if (!appendMaterialToLinkUnit(i, material, linkUnit.get(), argBlockIndex))
        {
            throw "Failed to append material to the link unit";
        }

        internalsInfo[i].argument_block_index = argBlockIndex;
    }

    mi::base::Handle<const mi::neuraylib::ITarget_code> targetCode(mBackend->translate_link_unit(linkUnit.get(), mContext.get()));
    mLogger->flushContextMessages(mContext.get());

    if (!targetCode)
    {
        throw "No target code";
    }

    std::stringstream ss;
    ss << "#include \"mdl_types.hlsl\"\n"
          "#include \"mdl_runtime.hlsl\"\n";

    ss << targetCode->get_code();

    _generateInOutSwitch(ss, SCATTERING_FUNC_NAME, "sample", "Bsdf_sample_data", materialCount);
    _generateInitSwitch(ss, SCATTERING_FUNC_NAME, materialCount);
    _generateInOutSwitch(ss, SCATTERING_FUNC_NAME, "pdf", "Bsdf_pdf_data", materialCount);

    _generateInOutSwitch(ss, SCATTERING_FUNC_NAME, "evaluate", "Bsdf_evaluate_data", materialCount);

    _generateInOutSwitch(ss, EMISSION_FUNC_NAME, "evaluate", "Edf_evaluate_data", materialCount);
    _generateInitSwitch(ss, EMISSION_FUNC_NAME, materialCount);

    _generateEdfIntensitySwitch(ss, materialCount);

    hlslSrc = ss.str();

    return targetCode;
}

bool MdlHlslCodeGen::appendMaterialToLinkUnit(uint32_t idx,
                                              const mi::neuraylib::ICompiled_material* compiledMaterial,
                                              mi::neuraylib::ILink_unit* linkUnit,
                                              mi::Size& argBlockIndex)
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
        mContext.get());

    mLogger->flushContextMessages(mContext.get());

    if (result == 0)
    {
        argBlockIndex = genFunctions[0].argument_block_index;
    }

    return result == 0;
}
} // namespace nevk

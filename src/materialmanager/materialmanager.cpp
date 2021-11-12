#include "materialmanager.h"

#include <dlfcn.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>
#include <MaterialXCore/Material.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

namespace mx = MaterialX;

namespace nevk
{
void MaterialManager::initMtlx(const char* mtlxlibPath)
{
    m_mtlxlibPath.append(mtlxlibPath);

    // Init shadergen.
    m_shaderGen = mx::MdlShaderGenerator::create();
    std::string target = m_shaderGen->getTarget();

    // MaterialX libs.
    m_stdLib = mx::createDocument();
    mx::FilePathVec libFolders;
    mx::loadLibraries(libFolders, m_mtlxlibPath, m_stdLib);

    // Color management.
    mx::DefaultColorManagementSystemPtr colorSystem = mx::DefaultColorManagementSystem::create(target);
    colorSystem->loadLibrary(m_stdLib);
    m_shaderGen->setColorManagementSystem(colorSystem);

    // Unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(target);
    unitSystem->loadLibrary(m_stdLib);

    mx::UnitConverterRegistryPtr unitRegistry = mx::UnitConverterRegistry::create();
    mx::UnitTypeDefPtr distanceTypeDef = m_stdLib->getUnitTypeDef("distance");
    unitRegistry->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = m_stdLib->getUnitTypeDef("angle");
    unitRegistry->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));

    unitSystem->setUnitConverterRegistry(unitRegistry);
    m_shaderGen->setUnitSystem(unitSystem);
}

mx::TypedElementPtr _FindSurfaceShaderElement(const mx::DocumentPtr& doc)
{
    // Find renderable element.
    std::vector<mx::TypedElementPtr> renderableElements;
    mx::findRenderableElements(doc, renderableElements);

    if (renderableElements.size() != 1)
    {
        return nullptr;
    }

    // Extract surface shader node.
    mx::TypedElementPtr renderableElement = renderableElements.at(0);
    mx::NodePtr node = renderableElement->asA<mx::Node>();

    if (node && node->getType() == mx::MATERIAL_TYPE_STRING)
    {
        std::vector<mx::NodePtr> shaderNodes = mx::getShaderNodes(node, mx::SURFACE_SHADER_TYPE_STRING); // unordered set in gatling ?
        if (!shaderNodes.empty())
        {
            renderableElement = *shaderNodes.begin();
        }
    }

    mx::ElementPtr surfaceElement = doc->getDescendant(renderableElement->getNamePath());
    if (!surfaceElement)
    {
        return nullptr;
    }

    return surfaceElement->asA<mx::TypedElement>();
}

bool MaterialManager::translate(const char* mtlxSrc, std::string& mdlSrc, std::string& subIdentifier)
{
    // Don't cache the context because it is thread-local.
    mx::GenContext context(m_shaderGen);
    context.registerSourceCodeSearchPath(m_mtlxlibPath);

    mx::GenOptions& contextOptions = context.getOptions();
    contextOptions.targetDistanceUnit = "meter";

    mx::ShaderPtr shader = nullptr;
    try
    {
        mx::DocumentPtr doc = mx::createDocument();
        doc->importLibrary(m_stdLib);
        mx::readFromXmlFile(doc, mtlxSrc);

        mx::TypedElementPtr element = _FindSurfaceShaderElement(doc);
        if (!element)
        {
            return false;
        }

        subIdentifier = element->getName();
        shader = m_shaderGen->generate(subIdentifier, element, context);
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Exception generating MDL code: %s\n", ex.what());
    }

    if (!shader)
    {
        return false;
    }

    mx::ShaderStage pixelStage = shader->getStage(mx::Stage::PIXEL);
    mdlSrc = pixelStage.getSourceCode();
    return true;
}

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

bool MaterialManager::initCodeGen()
{
    m_backend = mi::base::Handle<mi::neuraylib::IMdl_backend>(m_backendApi->get_backend(mi::neuraylib::IMdl_backend_api::MB_HLSL));
    if (!m_backend.is_valid_interface())
    {
        std::cerr << "HLSL backend not supported by MDL runtime" << std::endl;
        return false;
    }
    m_context = mi::base::Handle<mi::neuraylib::IMdl_execution_context>(m_factory->create_execution_context());

    return true;
}

bool MaterialManager::translate(std::string& hlslSrc) // todo add materials array
{
    linkedUnit = m_backend->create_link_unit(m_transaction.get(), m_context.get());

    if (!linkedUnit)
    {
        return false;
    }

    uint32_t materialCount = 1;//materials.size();
    for (uint32_t i = 0; i < materialCount; i++)
    {
        const mi::neuraylib::ICompiled_material* material = compiledMaterial.get();
        assert(material);

        if (!appendMaterialToLinkUnit(i, material))
        {
            return false;
        }
    }

    mi::base::Handle<const mi::neuraylib::ITarget_code> targetCode(m_backend->translate_link_unit(linkedUnit, m_context.get()));

    if (!targetCode)
    {
        return false;
    }
// mtlx to mdl ?
    if (targetCode->get_texture_count() > 0)
    {
        std::cout << targetCode->get_texture_count() << std::endl;
        std::cout << targetCode->get_texture(1) << std::endl;
        std::cout << targetCode->get_texture_url(1)<< std::endl;
        std::cerr << "Textures not supported, aborting\n"
                  << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << targetCode->get_code();

   /* _generateInOutSwitch(ss, SCATTERING_FUNC_NAME, "sample", "Bsdf_sample_data", materialCount);
    _generateInitSwitch(ss, SCATTERING_FUNC_NAME, materialCount);

    _generateInOutSwitch(ss, EMISSION_FUNC_NAME, "evaluate", "Edf_evaluate_data", materialCount);
    _generateInitSwitch(ss, EMISSION_FUNC_NAME, materialCount);

    _generateEdfIntensitySwitch(ss, materialCount);*/

    hlslSrc = ss.str();
    return true;
}

bool MaterialManager::appendMaterialToLinkUnit(uint32_t idx,
                                               const mi::neuraylib::ICompiled_material* compiledMaterial)
{
    std::string idxStr = std::to_string(idx);
    auto scatteringFuncName = std::string(SCATTERING_FUNC_NAME) + "_" + idxStr;
    auto emissionFuncName = std::string(EMISSION_FUNC_NAME) + "_" + idxStr;
    auto emissionIntensityFuncName = std::string(EMISSION_INTENSITY_FUNC_NAME) + "_" + idxStr;

    std::vector<mi::neuraylib::Target_function_description> genFunctions;
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.scattering", scatteringFuncName.c_str()));
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.emission.emission", emissionFuncName.c_str()));
    genFunctions.push_back(mi::neuraylib::Target_function_description("surface.emission.intensity", emissionIntensityFuncName.c_str()));

    mi::Sint32 result = linkedUnit->add_material(
        compiledMaterial,
        genFunctions.data(),
        genFunctions.size(),
        m_context.get());

    return result == 0;
}

const char* MODULE_PREFIX = "::nevk_"; // unused
std::atomic_uint32_t s_idCounter(0); // unused
std::string _makeModuleName(const std::string& identifier)
{
    uint32_t uniqueId = ++s_idCounter;
    return std::string(MODULE_PREFIX) + std::to_string(uniqueId) + "_" + identifier;
    //std::string moduleName = "::gun_metal"; // name of file ?
    //return "::SR_velvet";
}

bool MaterialManager::compileMaterial(const std::string& src, const std::string& identifier)
{
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(m_factory->create_execution_context());

    std::string moduleName = _makeModuleName(identifier);

    bool result = createModule(context.get(), moduleName.c_str(), src.c_str()) &&
                  createCompiledMaterial(context.get(), moduleName.c_str(), identifier);

    return result;
}

bool MaterialManager::createModule(mi::neuraylib::IMdl_execution_context* context, const char* moduleName, const char* mdlSrc)
{
     mi::Sint32 result = m_impExpApi->load_module_from_string(m_transaction.get(), moduleName, mdlSrc, context); // Note that this method expects the module name, not the name of the file containing the module.
    // mi::Sint32 result = m_impExpApi->load_module(m_transaction.get(), moduleName, context); // Note that this method expects the module name, not the name of the file containing the module.
    return result == 0 || result == 1;
}

bool MaterialManager::createCompiledMaterial(mi::neuraylib::IMdl_execution_context* context, const char* moduleName, const std::string& identifier)
{
    mi::base::Handle<const mi::IString> moduleDbName(m_factory->get_db_module_name(moduleName));
    mi::base::Handle<const mi::neuraylib::IModule> module(m_transaction->access<mi::neuraylib::IModule>(moduleDbName->get_c_str()));
    assert(module);

    std::string materialDbName = std::string(moduleDbName->get_c_str()) + "::" + identifier;
    mi::base::Handle<const mi::IArray> funcs(module->get_function_overloads(materialDbName.c_str(), (const mi::neuraylib::IExpression_list*)nullptr));
    if (funcs->get_length() == 0)
    {
        std::string errorMsg = std::string("Material with identifier ") + identifier + " not found in MDL module\n";
        std::cerr << errorMsg << std::endl;
        return false;
    }

    if (funcs->get_length() > 1)
    {
        std::string errorMsg = std::string("Ambigious material identifier ") + identifier + " for MDL module\n";
        std::cerr << errorMsg << std::endl;
        return false;
    }

    mi::base::Handle<const mi::IString> exactMaterialDbName(funcs->get_element<mi::IString>(0));
    mi::base::Handle<const mi::neuraylib::IMaterial_definition> matDefinition(m_transaction->access<mi::neuraylib::IMaterial_definition>(exactMaterialDbName->get_c_str()));
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
    compiledMaterial = mi::base::Handle<mi::neuraylib::ICompiled_material>(matInstance->create_compiled_material(flags, context));
    return true;
}


bool MaterialManager::initMaterial(const char* mtlxmdlPath)
{
    mi::base::Handle<mi::neuraylib::IMdl_configuration> config(m_neuray->get_api_component<mi::neuraylib::IMdl_configuration>());

    if (config->add_mdl_path(mtlxmdlPath)) // configure the MDL module search path
    {
        std::cerr << "MaterialX MDL file path not found, translation not possible" << std::endl;
        return false;
    }

    m_database = mi::base::Handle<mi::neuraylib::IDatabase>(m_neuray->get_api_component<mi::neuraylib::IDatabase>());
    mi::base::Handle<mi::neuraylib::IScope> scope(m_database->get_global_scope());
    m_transaction = mi::base::Handle<mi::neuraylib::ITransaction>(scope->create_transaction());

    m_factory = mi::base::Handle<mi::neuraylib::IMdl_factory>(m_neuray->get_api_component<mi::neuraylib::IMdl_factory>());
    m_impExpApi = mi::base::Handle<mi::neuraylib::IMdl_impexp_api>(m_neuray->get_api_component<mi::neuraylib::IMdl_impexp_api>());
    m_backendApi = mi::base::Handle<mi::neuraylib::IMdl_backend_api>(m_neuray->get_api_component<mi::neuraylib::IMdl_backend_api>());

    return true;
}

bool MaterialManager::initMDL(const char* resourcePath)
{
    if (!loadDso(resourcePath))
    {
        return false;
    }
    if (!loadNeuray())
    {
        return false;
    }
    return m_neuray->start() == 0;
}

bool MaterialManager::loadDso(const char* resourcePath)
{
    std::string dsoFilename = std::string(resourcePath) + std::string("/libmdl_sdk" MI_BASE_DLL_FILE_EXT);

#ifdef MI_PLATFORM_WINDOWS
    HMODULE handle = LoadLibraryA(dsoFilename.c_str());
    if (!handle)
    {
        LPTSTR buffer = NULL;
        LPCTSTR message = TEXT("unknown failure");
        DWORD error_code = GetLastError();
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                          0, error_code,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
        {
            message = buffer;
        }
        fprintf(stderr, "Failed to load library (%u): %s", error_code, message);
        if (buffer)
        {
            LocalFree(buffer);
        }
        return false;
    }
#else
    void* handle = dlopen(dsoFilename.c_str(), RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }
#endif

    m_dsoHandle = handle;
    return true;
}

bool MaterialManager::loadNeuray()
{
#ifdef MI_PLATFORM_WINDOWS
    void* symbol = GetProcAddress(reinterpret_cast<HMODULE>(m_dsoHandle), "mi_factory");
    if (!symbol)
    {
        LPTSTR buffer = NULL;
        LPCTSTR message = TEXT("unknown failure");
        DWORD error_code = GetLastError();
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                          0, error_code,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
        {
            message = buffer;
        }
        fprintf(stderr, "GetProcAddress error (%u): %s", error_code, message);
        if (buffer)
        {
            LocalFree(buffer);
        }
        return false;
    }
#else
    void* symbol = dlsym(m_dsoHandle, "mi_factory");
    if (!symbol)
    {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }
#endif

    m_neuray = mi::base::Handle<mi::neuraylib::INeuray>(mi::neuraylib::mi_factory<mi::neuraylib::INeuray>(symbol));
    if (m_neuray.is_valid_interface())
    {
        return true;
    }

    mi::base::Handle<mi::neuraylib::IVersion> version(mi::neuraylib::mi_factory<mi::neuraylib::IVersion>(symbol));
    if (!version)
    {
        fprintf(stderr, "Error: Incompatible library.\n");
    }
    else
    {
        fprintf(stderr, "Error: Library version %s does not match header version %s.\n", version->get_product_version(), MI_NEURAYLIB_PRODUCT_VERSION_STRING);
    }

    return false;
}

void MaterialManager::unloadDso()
{
#ifdef MI_PLATFORM_WINDOWS
    if (FreeLibrary(reinterpret_cast<HMODULE>(m_dsoHandle)))
    {
        return;
    }
    LPTSTR buffer = 0;
    LPCTSTR message = TEXT("unknown failure");
    DWORD error_code = GetLastError();
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                      0, error_code,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
    {
        message = buffer;
    }
    fprintf(stderr, "Failed to unload library (%u): %s", error_code, message);
    if (buffer)
    {
        LocalFree(buffer);
    }
#else
    if (dlclose(m_dsoHandle) != 0)
    {
        printf("%s\n", dlerror());
    }
#endif
}

} // namespace nevk

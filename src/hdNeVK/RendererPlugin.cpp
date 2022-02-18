#include "RendererPlugin.h"
#include "RenderDelegate.h"

#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/base/plug/plugin.h>
#include "pxr/base/plug/thisPlugin.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    HdRendererPluginRegistry::Define<HdNeVKRendererPlugin>();
}

HdNeVKRendererPlugin::HdNeVKRendererPlugin()
{
    PlugPluginPtr plugin = PLUG_THIS_PLUGIN;

    const std::string& resourcePath = plugin->GetResourcePath();
    printf("Resource path %s\n", resourcePath.c_str());
    std::string shaderPath = resourcePath + "/shaders";
    std::string mtlxmdlPath = resourcePath + "/mtlxmdl";
    std::string mtlxlibPath = resourcePath + "/mtlxlib";

    //m_translator = std::make_unique<MaterialNetworkTranslator>(mtlxlibPath);
    const char* envUSDPath = std::getenv("USD_PATH");
    if (!envUSDPath)
    {
        printf("Please, set USD_PATH variable\n");
        assert(0);
        m_isSupported = false;
    }
    else
    {
        std::string USDPath(envUSDPath);
        m_translator = std::make_unique<MaterialNetworkTranslator>(USDPath + "./libraries");
        m_isSupported = true;
    }
}

HdNeVKRendererPlugin::~HdNeVKRendererPlugin()
{
}

HdRenderDelegate* HdNeVKRendererPlugin::CreateRenderDelegate()
{
    HdRenderSettingsMap settingsMap;

    return new HdNeVKRenderDelegate(settingsMap, *m_translator);
}

HdRenderDelegate* HdNeVKRendererPlugin::CreateRenderDelegate(const HdRenderSettingsMap& settingsMap)
{
    return new HdNeVKRenderDelegate(settingsMap, *m_translator);
}

void HdNeVKRendererPlugin::DeleteRenderDelegate(HdRenderDelegate* renderDelegate)
{
    delete renderDelegate;
}

bool HdNeVKRendererPlugin::IsSupported() const
{
    return m_isSupported;
}

PXR_NAMESPACE_CLOSE_SCOPE

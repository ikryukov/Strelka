#pragma once

#include "ShaderManager.h"

#include <mi/mdl_sdk.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include "mdlHlslCodeGen.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"

namespace nevk
{

class MaterialManager
{
public:
    MaterialManager(TextureManager* texManager) : mTexManager(texManager) {
        runtime = new nevk::MdlRuntime();
        runtime->init(resourcePath.c_str(), pathso.c_str(), pathmdl.c_str());

        matCompiler = new nevk::MdlMaterialCompiler(*runtime);
        matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial);
        materials.push_back(compiledMaterial.get());

        codeGen = new MdlHlslCodeGen(mTexManager);
        codeGen->init(*runtime);
        targetHlsl = codeGen->translate(materials, hlslCode);
        codeGen->loadTextures(targetHlsl);

        createSampler();
    };

    std::string hlslCode;

    ~MaterialManager(){};

private:
    nevk::MdlHlslCodeGen* codeGen = nullptr;
    nevk::MdlMaterialCompiler* matCompiler = nullptr;
    nevk::MdlRuntime* runtime = nullptr;

    TextureManager* mTexManager = nullptr;

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string resourcePath = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/resources"; // todo: fix path
    // mdl -> hlsl
    std::string mdlSrc = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string ident = "carbon_composite";

    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    std::vector<const mi::neuraylib::ICompiled_material*> materials;

    mi::base::Handle<const mi::neuraylib::ITarget_code> targetHlsl;

    VkSampler mMaterialSampler;

    bool createSampler(){
        // sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        //VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mMaterialSampler);
        /*if (res != VK_SUCCESS)
        {
            // error
            assert(0);
        }*/
        
        return true;
    }
};
} // namespace nevk

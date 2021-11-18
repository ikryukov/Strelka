#include "materialmanager.h"

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>
#include <MaterialXCore/Material.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

namespace nevk
{
std::vector<uint8_t> MaterialManager::loadArgBlocks()
{
    /* for (int i = 0; i < materials.size(); i++)
     {
         mi::Size argLayoutIndex = mInternalsInfo[i].argument_block_index;
         if (argLayoutIndex != static_cast<mi::Size>(-1) &&
             argLayoutIndex < targetHlsl->get_argument_layout_count())
         {
             // argument block for class compilation parameter data
             mi::base::Handle<const mi::neuraylib::ITarget_argument_block> arg_block;
             {
                 // get the layout
                 mi::base::Handle<const mi::neuraylib::ITarget_value_layout> arg_layout(
                     targetHlsl->get_argument_block_layout(argLayoutIndex));

                 // for the first instances of the materials, the argument block already exists
                 // for further blocks new ones have to be created. To avoid special treatment,
                 // an new block is created for every material
                 mi::base::Handle<mi::neuraylib::ITarget_resource_callback> callback(
                     targetHlsl->create_resource_callback(this));

                 arg_block = targetHlsl->create_argument_block(
                     argLayoutIndex,
                     materials[i],
                     callback.get());

                 if (!arg_block)
                 {
                     std::cerr << ("Failed to create material argument block: ") << std::endl;
                     return false;
                 }
             }
             // create a buffer to provide those parameters to the shader
             size_t buffer_size = round_to_power_of_two(arg_block->get_size(), 4);
             argBlockData = std::vector<uint8_t>(buffer_size, 0);
             memcpy(argBlockData.data(), arg_block->get_data(), arg_block->get_size());
         }
     }*/

    mArgBlockData.resize(4);
    return mArgBlockData;
};

std::vector<uint8_t> MaterialManager::loadROData()
{
    size_t ro_data_seg_index = 0; // assuming one material per target code only
    if (mTargetHlsl->get_ro_data_segment_count() > 0)
    {
        const char* data = mTargetHlsl->get_ro_data_segment_data(ro_data_seg_index);
        size_t dataSize = mTargetHlsl->get_ro_data_segment_size(ro_data_seg_index);
        const char* name = mTargetHlsl->get_ro_data_segment_name(ro_data_seg_index);

        std::cerr << name << std::endl;

        mRoData.resize(dataSize);
        if (dataSize != 0)
        {
            memcpy(mRoData.data(), data, dataSize);
        }
    }

    if (mRoData.empty())
    {
        mRoData.resize(4);
    }

    return mRoData;
}

bool MaterialManager::createSampler()
{


    return true;
}
// Prepare the texture identified by the texture_index for use by the texture access functions
// on the GPU.
bool MaterialManager::prepare_texture(
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
        mLogger->message(mi::base::MESSAGE_SEVERITY_ERROR, "The example does not support tiled images!");
        return false;
    }

    if (tex_layers != 1)
    {
        mLogger->message(mi::base::MESSAGE_SEVERITY_ERROR, "The example does not support layered images!");
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
    mi::neuraylib::ITarget_code::Texture_shape texture_shape = code->get_texture_shape(texture_index);
    if (texture_shape == mi::neuraylib::ITarget_code::Texture_shape_2d)
    {
        mi::base::Handle<const mi::neuraylib::ITile> tile(canvas->get_tile());
        mi::Float32 const* data = static_cast<mi::Float32 const*>(tile->get_data());
        int cpp = image_api->get_components_per_pixel("Color");
        int bpc = image_api->get_bytes_per_component("Color");
        int bpp = cpp * bpc;
        uint32_t id = mTexManager->loadTextureMdl(data, tex_width, tex_height, image_type, std::to_string(texture_index));

        Mdl_resource_info info{};
        info.gpu_resource_array_start = id;
        mInfo.push_back(info);
    }

    return true;
}

bool MaterialManager::loadTextures(mi::base::Handle<const mi::neuraylib::ITarget_code>& targetCode)
{
    // Acquire image API needed to prepare the textures
    mi::base::Handle<mi::neuraylib::IImage_api> image_api(mNeuray->get_api_component<mi::neuraylib::IImage_api>());

    if (targetCode->get_texture_count() > 0)
    {
        for (int i = 1; i < targetCode->get_texture_count(); ++i)
        {
            mLogger->message(mi::base::MESSAGE_SEVERITY_INFO, targetCode->get_texture_url(i));
            prepare_texture(mTransaction, image_api, targetCode, i);
        }
    }
    else
    {
        mInfo.resize(1);
        return false;
    }

    return true;
}
} // namespace nevk

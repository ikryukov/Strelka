// https://github.com/NVIDIA/MDL-SDK/blob/master/examples/mdl_sdk/dxr/content/mdl_renderer_runtime.hlsl

#include "materials.h"
#include "bindless.h"

// per target data
ByteAddressBuffer mdl_ro_data_segment;
// per material data
// - argument block contains dynamic parameter data exposed in class compilation mode
ByteAddressBuffer mdl_argument_block;
// - resource infos map resource IDs, generated by the SDK, to actual buffer views
StructuredBuffer<Mdl_resource_info> mdl_resource_infos;
// - texture views, unbound and overlapping for 2D and 3D resources
Texture2D mdl_textures_2d[BINDLESS_TEXTURE_COUNT]; // mac doesn't support unsized arrays
// Texture3D mdl_textures_3d[]; // TODO: add support and check with material

// global samplers
SamplerState mdl_sampler_tex;

// ------------------------------------------------------------------------------------------------
// Argument block access for dynamic parameters in class compilation mode
// ------------------------------------------------------------------------------------------------

float mdl_read_argblock_as_float(uint offs)
{
    return asfloat(mdl_argument_block.Load(offs));
}

double mdl_read_argblock_as_double(uint offs)
{
    return asdouble(mdl_argument_block.Load(offs), mdl_argument_block.Load(offs + 4));
}

int mdl_read_argblock_as_int(uint offs)
{
    return asint(mdl_argument_block.Load(offs));
}

uint mdl_read_argblock_as_uint(uint offs)
{
    return mdl_argument_block.Load(offs);
}

bool mdl_read_argblock_as_bool(uint offs)
{
    uint val = mdl_argument_block.Load(offs & ~3);
    return (val & (0xff << (8 * (offs & 3)))) != 0;
}


float mdl_read_rodata_as_float(uint offs)
{
    return asfloat(mdl_ro_data_segment.Load(offs));
}

double mdl_read_rodata_as_double(uint offs)
{
    return asdouble(mdl_ro_data_segment.Load(offs), mdl_ro_data_segment.Load(offs + 4));
}

int mdl_read_rodata_as_int(uint offs)
{
    return asint(mdl_ro_data_segment.Load(offs));
}

uint mdl_read_rodata_as_uint(uint offs)
{
    return mdl_ro_data_segment.Load(offs);
}

bool mdl_read_rodata_as_bool(uint offs)
{
    uint val = mdl_ro_data_segment.Load(offs & ~3);
    return (val & (0xff << (8 * (offs & 3)))) != 0;
}

// ------------------------------------------------------------------------------------------------
// Texturing functions, check if valid
// ------------------------------------------------------------------------------------------------

// corresponds to ::tex::texture_isvalid(uniform texture_2d tex)
// corresponds to ::tex::texture_isvalid(uniform texture_3d tex)
// corresponds to ::tex::texture_isvalid(uniform texture_cube tex) // not supported by this example
// corresponds to ::tex::texture_isvalid(uniform texture_ptex tex) // not supported by this example
bool tex_texture_isvalid(uint tex)
{
    // assuming that there is no indexing out of bounds of the resource_infos and the view arrays
    return tex != 0; // invalid texture
}

// helper function to realize wrap and crop.
// Out of bounds case for TEX_WRAP_CLIP must already be handled.
float apply_wrap_and_crop(
    float coord,
    int wrap,
    float2 crop,
    int res)
{
    if (wrap != TEX_WRAP_REPEAT || any(crop != float2(0, 1)))
    {
        if (wrap == TEX_WRAP_REPEAT)
        {
            coord -= floor(coord);
        }
        else
        {
            if (wrap == TEX_WRAP_MIRRORED_REPEAT)
            {
                float floored_val = floor(coord);
                if ((int(floored_val) & 1) != 0)
                    coord = 1 - (coord - floored_val);
                else
                    coord -= floored_val;
            }
            float inv_hdim = 0.5f / float(res);
            coord = clamp(coord, inv_hdim, 1.f - inv_hdim);
        }
        coord = coord * (crop.y - crop.x) + crop.x;
    }
    return coord;
}

// Modify texture coordinates to get better texture filtering,
// see http://www.iquilezles.org/www/articles/texture/texture.htm
float2 apply_smootherstep_filter(float2 uv, uint2 size)
{
    float2 res;
    res = uv * size + 0.5f;

    float u_i = floor(res.x), v_i = floor(res.y);
    float u_f = res.x - u_i;
    float v_f = res.y - v_i;
    u_f = u_f * u_f * u_f * (u_f * (u_f * 6.f - 15.f) + 10.f);
    v_f = v_f * v_f * v_f * (v_f * (v_f * 6.f - 15.f) + 10.f);
    res.x = u_i + u_f;
    res.y = v_i + v_f;

    res = (res - 0.5f) / size;
    return res;
}
// ------------------------------------------------------------------------------------------------
// Texturing functions, 2D
// ------------------------------------------------------------------------------------------------

uint2 tex_res_2d(uint tex, int2 uv_tile, float frame)
{
    if (tex == 0) return uint2(0, 0); // invalid texture

    // fetch the infos about this resource
    Mdl_resource_info info = mdl_resource_infos[tex - 1]; // assuming this is in bounds

    int array_index = info.gpu_resource_array_start;
    if (array_index < 0) return uint2(0, 0); // out of bounds or no UDIM tile

    uint2 res;
    mdl_textures_2d[NonUniformResourceIndex(array_index)].GetDimensions(res.x, res.y);
    return res;
}

// corresponds to ::tex::width(uniform texture_2d tex, int2 uv_tile, float frame)
uint tex_width_2d(uint tex, int2 uv_tile, float frame)
{
    return tex_res_2d(tex, uv_tile, frame).x;
}

// corresponds to ::tex::height(uniform texture_2d tex, int2 uv_tile)
uint tex_height_2d(uint tex, int2 uv_tile, float frame)
{
    return tex_res_2d(tex, uv_tile, frame).y;
}

// corresponds to ::tex::lookup_float4(uniform texture_2d tex, float2 coord, ...)
float4 tex_lookup_float4_2d(
    uint tex,
    float2 coord,
    int wrap_u,
    int wrap_v,
    float2 crop_u,
    float2 crop_v,
    float frame)
{
    if (tex == 0) return float4(0, 0, 0, 0); // invalid texture

    // fetch the infos about this resource
    Mdl_resource_info info = mdl_resource_infos[tex - 1]; // assuming this is in bounds

    // handle UDIM and/or get texture array index
    int array_index = info.gpu_resource_array_start;
    if (array_index < 0) return float4(0, 0, 0, 0); // out of bounds or no UDIM tile

    if (wrap_u == TEX_WRAP_CLIP && (coord.x < 0.0 || coord.x >= 1.0))
        return float4(0, 0, 0, 0);
    if (wrap_v == TEX_WRAP_CLIP && (coord.y < 0.0 || coord.y >= 1.0))
        return float4(0, 0, 0, 0);

    uint width, height;
    mdl_textures_2d[NonUniformResourceIndex(array_index)].GetDimensions(width, height);
    coord.x = apply_wrap_and_crop(coord.x, wrap_u, crop_u, width);
    coord.y = apply_wrap_and_crop(coord.y, wrap_v, crop_v, height);

    coord = apply_smootherstep_filter(coord, uint2(width, height));

    // Note, since we don't have ddx and ddy in the compute pipeline, TextureObject::Sample() is not
    // available, we use SampleLevel instead and go for the most detailed level. Therefore, we don't
    // need mipmaps. Manual mip level computation is possible though.
    return mdl_textures_2d[NonUniformResourceIndex(array_index)].SampleLevel(
        mdl_sampler_tex, coord, /*lod=*/ 0.0f, /*offset=*/ int2(0, 0));
}

float3 tex_lookup_float3_2d(uint tex, float2 coord, int wrap_u, int wrap_v, float2 crop_u, float2 crop_v, float frame)
{
    return tex_lookup_float4_2d(tex, coord, wrap_u, wrap_v, crop_u, crop_v, frame).xyz;
}

float3 tex_lookup_color_2d(uint tex, float2 coord, int wrap_u, int wrap_v, float2 crop_u, float2 crop_v, float frame)
{
    return tex_lookup_float4_2d(tex, coord, wrap_u, wrap_v, crop_u, crop_v, frame).xyz;
}

float2 tex_lookup_float2_2d(uint tex, float2 coord, int wrap_u, int wrap_v, float2 crop_u, float2 crop_v, float frame)
{
    return tex_lookup_float4_2d(tex, coord, wrap_u, wrap_v, crop_u, crop_v, frame).xy;
}

float tex_lookup_float_2d(uint tex, float2 coord, int wrap_u, int wrap_v, float2 crop_u, float2 crop_v, float frame)
{
    return tex_lookup_float4_2d(tex, coord, wrap_u, wrap_v, crop_u, crop_v, frame).x;
}

// corresponds to ::tex::texel_float4(uniform texture_2d tex, float2 coord, int2 uv_tile)
float4 tex_texel_float4_2d(
    uint tex,
    int2 coord,
    int2 uv_tile,
    float frame)
{
    if (tex == 0) return float4(0, 0, 0, 0); // invalid texture

    // fetch the infos about this resource
    Mdl_resource_info info = mdl_resource_infos[tex - 1]; // assuming this is in bounds

    // handle UDIM and/or get texture array index
    int array_index = info.gpu_resource_array_start;
    if (array_index < 0) return float4(0, 0, 0, 0); // out of bounds or no UDIM tile

    uint2 res;
    mdl_textures_2d[NonUniformResourceIndex(array_index)].GetDimensions(res.x, res.y);
    if (0 > coord.x || res.x <= coord.x || 0 > coord.y || res.y <= coord.y)
        return float4(0, 0, 0, 0); // out of bounds

    return mdl_textures_2d[NonUniformResourceIndex(array_index)].Load(int3(coord, /*mipmaplevel=*/ 0));
}

float3 tex_texel_float3_2d(uint tex, int2 coord, int2 uv_tile, float frame)
{
    return tex_texel_float4_2d(tex, coord, uv_tile, frame).xyz;
}

float3 tex_texel_color_2d(uint tex, int2 coord, int2 uv_tile, float frame)
{
    return tex_texel_float3_2d(tex, coord, uv_tile, frame);
}

float2 tex_texel_float2_2d(uint tex, int2 coord, int2 uv_tile, float frame)
{
    return tex_texel_float4_2d(tex, coord, uv_tile, frame).xy;
}

float tex_texel_float_2d(uint tex, int2 coord, int2 uv_tile, float frame)
{
    return tex_texel_float4_2d(tex, coord, uv_tile, frame).x;
}

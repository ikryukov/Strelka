#include "RenderBuffer.h"

#include <pxr/base/gf/vec3i.h>

PXR_NAMESPACE_OPEN_SCOPE

HdStrelkaRenderBuffer::HdStrelkaRenderBuffer(const SdfPath& id, oka::SharedContext* ctx)
    : HdRenderBuffer(id), mCtx(ctx)
{
    m_isMapped = false;
    m_isConverged = false;
    m_bufferMem = nullptr;
}

HdStrelkaRenderBuffer::~HdStrelkaRenderBuffer()
{
}

bool HdStrelkaRenderBuffer::Allocate(const GfVec3i& dimensions,
                                     HdFormat format,
                                     bool multiSampled)
{
    if (dimensions[2] != 1)
    {
        return false;
    }

    m_width = dimensions[0];
    m_height = dimensions[1];
    m_format = format;
    m_isMultiSampled = multiSampled;

    size_t size = m_width * m_height * HdDataSizeOfFormat(m_format);

    m_bufferMem = realloc(m_bufferMem, size);

    if (!m_bufferMem)
    {
        return false;
    }

    if (mResult)
    {
        mCtx->mResManager->destroyImage(mResult);
    }
    mResult = mCtx->mResManager->createImage(m_width, m_height, VK_FORMAT_R32G32B32A32_SFLOAT,
                                             VK_IMAGE_TILING_OPTIMAL,
                                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "HdOkaRenderBuffer");
    mCtx->mTextureManager->transitionImageLayout(mCtx->mResManager->getVkImage(mResult), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    return true;
}

unsigned int HdStrelkaRenderBuffer::GetWidth() const
{
    return m_width;
}

unsigned int HdStrelkaRenderBuffer::GetHeight() const
{
    return m_height;
}

unsigned int HdStrelkaRenderBuffer::GetDepth() const
{
    return 1u;
}

HdFormat HdStrelkaRenderBuffer::GetFormat() const
{
    return m_format;
}

bool HdStrelkaRenderBuffer::IsMultiSampled() const
{
    return m_isMultiSampled;
}

VtValue HdStrelkaRenderBuffer::GetResource(bool multiSampled) const
{
    return VtValue((uint8_t*) mResult);
}

bool HdStrelkaRenderBuffer::IsConverged() const
{
    return m_isConverged;
}

void HdStrelkaRenderBuffer::SetConverged(bool converged)
{
    m_isConverged = converged;
}

void* HdStrelkaRenderBuffer::Map()
{
    m_isMapped = true;

    return m_bufferMem;
}

bool HdStrelkaRenderBuffer::IsMapped() const
{
    return m_isMapped;
}

void HdStrelkaRenderBuffer::Unmap()
{
    m_isMapped = false;
}

void HdStrelkaRenderBuffer::Resolve()
{
}

void HdStrelkaRenderBuffer::_Deallocate()
{
    free(m_bufferMem);
}

PXR_NAMESPACE_CLOSE_SCOPE

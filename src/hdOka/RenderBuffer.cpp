#include "RenderBuffer.h"

#include <pxr/base/gf/vec3i.h>

PXR_NAMESPACE_OPEN_SCOPE

HdOkaRenderBuffer::HdOkaRenderBuffer(const SdfPath& id, oka::SharedContext* ctx)
    : HdRenderBuffer(id), mCtx(ctx)
{
    m_isMapped = false;
    m_isConverged = false;
    m_bufferMem = nullptr;
}

HdOkaRenderBuffer::~HdOkaRenderBuffer()
{
}

bool HdOkaRenderBuffer::Allocate(const GfVec3i& dimensions,
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

unsigned int HdOkaRenderBuffer::GetWidth() const
{
    return m_width;
}

unsigned int HdOkaRenderBuffer::GetHeight() const
{
    return m_height;
}

unsigned int HdOkaRenderBuffer::GetDepth() const
{
    return 1u;
}

HdFormat HdOkaRenderBuffer::GetFormat() const
{
    return m_format;
}

bool HdOkaRenderBuffer::IsMultiSampled() const
{
    return m_isMultiSampled;
}

VtValue HdOkaRenderBuffer::GetResource(bool multiSampled) const
{
    return VtValue((uint8_t*) mResult);
}

bool HdOkaRenderBuffer::IsConverged() const
{
    return m_isConverged;
}

void HdOkaRenderBuffer::SetConverged(bool converged)
{
    m_isConverged = converged;
}

void* HdOkaRenderBuffer::Map()
{
    m_isMapped = true;

    return m_bufferMem;
}

bool HdOkaRenderBuffer::IsMapped() const
{
    return m_isMapped;
}

void HdOkaRenderBuffer::Unmap()
{
    m_isMapped = false;
}

void HdOkaRenderBuffer::Resolve()
{
}

void HdOkaRenderBuffer::_Deallocate()
{
    free(m_bufferMem);
}

PXR_NAMESPACE_CLOSE_SCOPE

#include "RenderBuffer.h"

#include <pxr/base/gf/vec3i.h>

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKRenderBuffer::HdNeVKRenderBuffer(const SdfPath& id)
    : HdRenderBuffer(id)
{
    m_isMapped = false;
    m_isConverged = false;
    m_bufferMem = nullptr;
}

HdNeVKRenderBuffer::~HdNeVKRenderBuffer()
{
}

bool HdNeVKRenderBuffer::Allocate(const GfVec3i& dimensions,
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

    return true;
}

unsigned int HdNeVKRenderBuffer::GetWidth() const
{
    return m_width;
}

unsigned int HdNeVKRenderBuffer::GetHeight() const
{
    return m_height;
}

unsigned int HdNeVKRenderBuffer::GetDepth() const
{
    return 1u;
}

HdFormat HdNeVKRenderBuffer::GetFormat() const
{
    return m_format;
}

bool HdNeVKRenderBuffer::IsMultiSampled() const
{
    return m_isMultiSampled;
}

bool HdNeVKRenderBuffer::IsConverged() const
{
    return m_isConverged;
}

void HdNeVKRenderBuffer::SetConverged(bool converged)
{
    m_isConverged = converged;
}

void* HdNeVKRenderBuffer::Map()
{
    m_isMapped = true;

    return m_bufferMem;
}

bool HdNeVKRenderBuffer::IsMapped() const
{
    return m_isMapped;
}

void HdNeVKRenderBuffer::Unmap()
{
    m_isMapped = false;
}

void HdNeVKRenderBuffer::Resolve()
{
}

void HdNeVKRenderBuffer::_Deallocate()
{
    free(m_bufferMem);
}

PXR_NAMESPACE_CLOSE_SCOPE

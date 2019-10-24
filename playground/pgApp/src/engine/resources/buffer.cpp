#include "../engine.h"

pgBuffer::pgBuffer(uint32_t stride, uint32_t count, Diligent::IBuffer* buffer)
    : m_uiCount(count), m_uiStride(stride), m_BindFlags(0), m_bIsBound(false)
{
    if (buffer) {
        m_pBuffer.Attach(buffer);
        m_pBuffer->AddRef();
    }
}

pgBuffer::~pgBuffer()
{
    m_pBuffer.Release();
    m_pBuffer.Detach();
}

Diligent::IBuffer* pgBuffer::GetBuffer()
{
    return m_pBuffer;
}


uint32_t pgBuffer::GetCount() const
{
    return m_uiCount;
}

uint32_t pgBuffer::GetSize() const
{
    return m_uiCount * m_uiStride;
}


pgBuffer::BufferType pgBuffer::GetType() const
{
    return pgBuffer::Unknown;
}

Diligent::IBufferView* pgBuffer::GetUnorderedAccessView()
{
    auto buffer = m_pBuffer;

    auto uav = buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
    return uav;
}

Diligent::IBufferView* pgBuffer::GetShaderResourceView()
{
    auto buffer = m_pBuffer;

    auto srv = buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
    return srv;
}

// Bind the buffer for rendering.
bool pgBuffer::Bind(unsigned int slot, Shader::ShaderType shaderType,
                    ShaderParameter::Type parameterType)
{
    auto buffer = m_pBuffer;

    Diligent::Uint32 offset[] = { 0 };
    Diligent::IBuffer* pBuffs[] = { buffer };
    const uint32_t buffs = 1;

    pgApp::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset,
                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                   Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

    return true;
}

// Unbind the buffer for rendering.
void pgBuffer::UnBind(unsigned int id, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType)
{
    //
}

void pgBuffer::Copy(std::shared_ptr<pgBuffer> other)
{
    //
}

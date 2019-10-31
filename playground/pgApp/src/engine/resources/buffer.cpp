#include "../engine.h"

namespace ade
{

Buffer::Buffer(uint32_t stride, uint32_t count, Diligent::IBuffer* buffer)
    : m_uiCount(count), m_uiStride(stride), m_BindFlags(0), m_bIsBound(false)
{
    if (buffer) {
        m_pBuffer.Attach(buffer);
        m_pBuffer->AddRef();
    }
}

Buffer::~Buffer()
{
    m_pBuffer.Release();
    m_pBuffer.Detach();
}

Diligent::IBuffer* Buffer::GetBuffer()
{
    return m_pBuffer;
}


uint32_t Buffer::GetCount() const
{
    return m_uiCount;
}

uint32_t Buffer::GetSize() const
{
    return m_uiCount * m_uiStride;
}


Buffer::BufferType Buffer::GetType() const
{
    return Buffer::Unknown;
}

void Buffer::SetBufferFormat(Diligent::VALUE_TYPE ValueType)
{
    m_valueType = ValueType;
}

Diligent::IBufferView* Buffer::GetUnorderedAccessView()
{
    if (m_valueType != Diligent::VALUE_TYPE::VT_UNDEFINED) {
        Diligent::BufferViewDesc ViewDesc;
        ViewDesc.ViewType = Diligent::BUFFER_VIEW_UNORDERED_ACCESS;
        ViewDesc.Format.ValueType = m_valueType;
        ViewDesc.Format.NumComponents = 1;

        Diligent::IBufferView* uav = 0;
        m_pBuffer->CreateView(ViewDesc, &uav);

        return uav;
    } else {
        auto buffer = m_pBuffer;

        auto uav = buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
        return uav;
    }
}

Diligent::IBufferView* Buffer::GetShaderResourceView()
{
    if (m_valueType != Diligent::VALUE_TYPE::VT_UNDEFINED) {
        Diligent::BufferViewDesc ViewDesc;
        ViewDesc.ViewType = Diligent::BUFFER_VIEW_SHADER_RESOURCE;
        ViewDesc.Format.ValueType = m_valueType;
        ViewDesc.Format.NumComponents = 1;

        Diligent::IBufferView* srv = 0;
        m_pBuffer->CreateView(ViewDesc, &srv);

        return srv;
    } else {
        auto buffer = m_pBuffer;

        auto srv = buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
        return srv;
    }
}

// Bind the buffer for rendering.
bool Buffer::Bind(unsigned int slot, Shader::ShaderType shaderType,
                  ShaderParameter::Type parameterType)
{
    auto buffer = m_pBuffer;

    Diligent::Uint32 offset[] = { 0 };
    Diligent::IBuffer* pBuffs[] = { buffer };
    const uint32_t buffs = 1;

    App::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset,
                                 Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                 Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

    return true;
}

// Unbind the buffer for rendering.
void Buffer::UnBind(unsigned int id, Shader::ShaderType shaderType,
                    ShaderParameter::Type parameterType)
{
    //
}

void Buffer::Copy(std::shared_ptr<Buffer> other)
{
    //
}

}    // namespace ade
#include "../engine.h"

namespace ade
{

VertexBuffer::VertexBuffer(const void* data, uint32_t count, uint32_t stride, bool bSRV,
                           Diligent::VALUE_TYPE ValueType, CPUAccess cpuAccess, bool bUAV)
    : base(stride, count, nullptr)
{
    Diligent::BufferDesc BuffDesc;
    BuffDesc.Name = "VertexBuffer";

    if (bSRV || bUAV || cpuAccess != CPUAccess::None) {
        BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;

        if ((cpuAccess & CPUAccess::Read) != 0) {
            BuffDesc.Usage = Diligent::USAGE_STAGING;
            BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE | Diligent::CPU_ACCESS_READ;
        } else if ((cpuAccess & CPUAccess::Write) != 0) {
            BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
            BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
            BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        } else {
            BuffDesc.Usage = Diligent::USAGE_DEFAULT;
            BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

            if (bUAV) {
                BuffDesc.BindFlags |= Diligent::BIND_UNORDERED_ACCESS;
            }
        }

        CHECK_ERR(ValueType == Diligent::VALUE_TYPE::VT_FLOAT32, "only float is supported");
        uint8_t componentsCount = (uint8_t)stride / sizeof(float);
        SetBufferFormat(ValueType, componentsCount);

        BuffDesc.ElementByteStride = stride;
    } else {
        bool bDynamic = !data;
        if (bDynamic) {
            BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
            BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        } else {
            BuffDesc.Usage = Diligent::USAGE_STATIC;
            // BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        }
    }

    BuffDesc.BindFlags |= Diligent::BIND_VERTEX_BUFFER;

    BuffDesc.uiSizeInBytes = stride * count;

    Diligent::BufferData VBData;
    VBData.pData = data;
    VBData.DataSize = stride * count;

    App::s_device->CreateBuffer(BuffDesc, &VBData, &m_pBuffer);
}

VertexBuffer::~VertexBuffer()
{
    //
}

bool VertexBuffer::Bind(uint32_t slot, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType)
{
    auto buffer = m_pBuffer;

    Diligent::Uint32 offset[] = { 0 };
    Diligent::IBuffer* pBuffs[] = { buffer };
    const uint32_t buffs = 1;

    App::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset,
                                 Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                 Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

    return base::Bind(slot, shaderType, parameterType);
}

void VertexBuffer::UnBind(uint32_t slot, Shader::ShaderType shaderType,
                          ShaderParameter::Type parameterType)
{
    base::UnBind(slot, shaderType, parameterType);
}

void VertexBuffer::Copy(std::shared_ptr<VertexBuffer> other) {}

Buffer::BufferType VertexBuffer::GetType() const
{
    return Buffer::VertexBuffer;
}

uint32_t VertexBuffer::GetElementCount() const
{
    return m_uiCount;
}

void VertexBuffer::Set(const void* data, size_t size) {}

}    // namespace ade
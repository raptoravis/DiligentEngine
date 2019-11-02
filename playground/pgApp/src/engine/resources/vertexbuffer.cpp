#include "../engine.h"

namespace ade
{

VertexBuffer::VertexBuffer(const void* data, uint32_t count, uint32_t stride,
                                   CPUAccess cpuAccess, bool bUAV)
    : base(stride, count, nullptr)
{
    Diligent::BufferDesc BuffDesc;
    BuffDesc.Name = "VertexBuffer";

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

    BuffDesc.ElementByteStride = stride;
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

bool VertexBuffer::Bind(uint32_t ID, Shader::ShaderType shaderType,
                            ShaderParameter::Type parameterType)
{
    return true;
}

void VertexBuffer::UnBind(uint32_t ID, Shader::ShaderType shaderType,
                              ShaderParameter::Type parameterType)
{
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
#include "../engine.h"

namespace ade
{

StructuredBuffer::StructuredBuffer(const void* data, uint32_t count, uint32_t stride,
                                   CPUAccess cpuAccess, bool bUAV)
    : base(stride, count, nullptr)
{
    Diligent::BufferDesc BuffDesc;
    BuffDesc.Name = "StructuredBuffer";

    BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;

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

StructuredBuffer::~StructuredBuffer()
{
    //
}

bool StructuredBuffer::Bind(unsigned int ID, Shader::ShaderType shaderType,
                            ShaderParameter::Type parameterType)
{
    return true;
}

void StructuredBuffer::UnBind(unsigned int ID, Shader::ShaderType shaderType,
                              ShaderParameter::Type parameterType)
{
}

void StructuredBuffer::SetData(void* data, size_t elementSize, size_t offset, size_t numElements)
{
    struct dummy_t {
        char m;
    };

    size_t size = elementSize * numElements;

    Diligent::MapHelper<dummy_t> buffer(App::s_ctx, GetBuffer(), Diligent::MAP_WRITE,
                                        Diligent::MAP_FLAG_DISCARD);
    auto p = (&buffer->m + offset);
    memcpy((char*)p, data, size);
}

void StructuredBuffer::Copy(std::shared_ptr<StructuredBuffer> other) {}

void StructuredBuffer::Clear() {}

Buffer::BufferType StructuredBuffer::GetType() const
{
    return Buffer::StructuredBuffer;
}

unsigned int StructuredBuffer::GetElementCount() const
{
    return m_uiCount;
}

}    // namespace ade
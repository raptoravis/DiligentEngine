#include "../engine.h"

namespace ade
{

IndexBuffer::IndexBuffer(const void* data, uint32_t count, uint32_t stride, bool bSRV,
                         Diligent::VALUE_TYPE ValueType, CPUAccess cpuAccess, bool bUAV)
    : base(stride, count, nullptr)
{
    Diligent::BufferDesc BuffDesc;
    BuffDesc.Name = "IndexBuffer";

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

        CHECK_ERR(ValueType == Diligent::VALUE_TYPE::VT_UINT32, "only uint32 is supported");
        uint8_t componentsCount = (uint8_t)stride / sizeof(uint32_t);
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

	if ((cpuAccess & CPUAccess::Read) != 0) {
		// bind nothing when staging
    } else {
        BuffDesc.BindFlags |= Diligent::BIND_INDEX_BUFFER;
    }

    BuffDesc.uiSizeInBytes = stride * count;

    Diligent::BufferData VBData;
    VBData.pData = data;
    VBData.DataSize = stride * count;

    App::s_device->CreateBuffer(BuffDesc, &VBData, &m_pBuffer);
}

IndexBuffer::~IndexBuffer()
{
    //
}

bool IndexBuffer::Bind(uint32_t ID, Shader::ShaderType shaderType,
                       ShaderParameter::Type parameterType)
{
    return base::Bind(ID, shaderType, parameterType);
}

void IndexBuffer::UnBind(uint32_t ID, Shader::ShaderType shaderType,
                         ShaderParameter::Type parameterType)
{
    base::UnBind(ID, shaderType, parameterType);
}

void IndexBuffer::Copy(std::shared_ptr<IndexBuffer> other) {}

Buffer::BufferType IndexBuffer::GetType() const
{
    return Buffer::IndexBuffer;
}

uint32_t IndexBuffer::GetElementCount() const
{
    return m_uiCount;
}

void IndexBuffer::Set(const void* data, size_t size) {}


}    // namespace ade
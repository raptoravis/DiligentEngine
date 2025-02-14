#include "../engine.h"

namespace ade
{

ConstantBuffer::ConstantBuffer(uint32_t size, void* data) : base(size, 1, nullptr)
{
    CreateUniformBuffer(App::s_device, size, "VS constants CB", &m_pBuffer,
                        Diligent::USAGE_DYNAMIC, Diligent::BIND_UNIFORM_BUFFER,
                        Diligent::CPU_ACCESS_WRITE, data);
}

ConstantBuffer::~ConstantBuffer() {}


Buffer::BufferType ConstantBuffer::GetType() const
{
    return Buffer::ConstantBuffer;
}

uint32_t ConstantBuffer::GetElementCount() const
{
    return 1;
}

bool ConstantBuffer::Bind(uint32_t id, Shader::ShaderType shaderType,
                          ShaderParameter::Type parameterType)
{
    return true;
}

void ConstantBuffer::UnBind(uint32_t id, Shader::ShaderType shaderType,
                            ShaderParameter::Type parameterType)
{
    //
}

// Copy the contents of a buffer to this one.
// Buffers must be the same size.
void ConstantBuffer::Copy(std::shared_ptr<ConstantBuffer> other) {}

// Implementations must provide this method.
void ConstantBuffer::Set(const void* data, size_t size)
{
    {
        // Map the buffer and write current world-view-projection matrix
        struct dummy_t {
            char m;
        };
        Diligent::MapHelper<dummy_t> CBConstants(App::s_ctx, GetBuffer(), Diligent::MAP_WRITE,
                                                 Diligent::MAP_FLAG_DISCARD);
        auto p = &CBConstants->m;
        memcpy((char*)p, data, size);
    }
}

}    // namespace ade
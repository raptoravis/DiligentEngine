#include "../engine.h"

ConstantBuffer::ConstantBuffer(uint32_t size, void* data)
	: base(size)
{
	// Create a vertex buffer that stores cube vertices
	Diligent::BufferDesc VertBuffDesc;
	VertBuffDesc.Name = "Float vertex buffer";
	VertBuffDesc.Usage = Diligent::USAGE_STATIC;
	VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = size;

	Diligent::BufferData VBData;
	VBData.pData = data;
	VBData.DataSize = size;

	pgApp::s_device->CreateBuffer(VertBuffDesc, &VBData, &m_pBuffer);
}

ConstantBuffer::~ConstantBuffer() {

}


pgBuffer::BufferType ConstantBuffer::GetType() const
{
    return pgBuffer::ConstantBuffer;
}

unsigned int ConstantBuffer::GetElementCount() const
{
    return 1;
}

bool ConstantBuffer::Bind(unsigned int id, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	return true;
}

void ConstantBuffer::UnBind(unsigned int id, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	//
}

// Copy the contents of a buffer to this one.
// Buffers must be the same size.
void ConstantBuffer::Copy(std::shared_ptr<ConstantBuffer> other) {

}

// Implementations must provide this method.
void ConstantBuffer::Set(const void* data, size_t size) {

}

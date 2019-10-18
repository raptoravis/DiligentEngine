#include "engine.h"

StructuredBuffer::StructuredBuffer(const void* data, uint32_t count,
	uint32_t stride, CPUAccess cpuAccess, bool bUAV)
	: base(stride * count)
{
	Diligent::BufferDesc BuffDesc;
	BuffDesc.Name = "Lights StructuredBuffer";
	BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
	BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	BuffDesc.ElementByteStride = stride;
	BuffDesc.uiSizeInBytes = stride * count;

	Diligent::BufferData VBData;
	VBData.pData = data;
	VBData.DataSize = stride * count;

	pgApp::s_device->CreateBuffer(BuffDesc, &VBData, &m_pBuffer);
}

StructuredBuffer::~StructuredBuffer() {
	//
}

bool StructuredBuffer::Bind(unsigned int ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType)
{
	return true;
}

void StructuredBuffer::UnBind(unsigned int ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType)
{
}

void StructuredBuffer::SetData(void* data, size_t elementSize, size_t offset, size_t numElements)
{
}

void StructuredBuffer::Copy(std::shared_ptr<StructuredBuffer> other)
{
}

void StructuredBuffer::Clear()
{
}

pgBuffer::BufferType StructuredBuffer::GetType() const
{
	return pgBuffer::StructuredBuffer;
}

unsigned int StructuredBuffer::GetElementCount() const
{
	return m_uiCount;
}

#include "engine.h"

#include <limits>


ShaderParameter::ShaderParameter()
	: m_uiSlotID(std::numeric_limits<uint32_t>::max())
	, m_ParameterType(Type::Invalid)
{}

ShaderParameter::ShaderParameter(const std::string& name, uint32_t slotID, const std::string& shaderType, Type parameterType)
	: m_Name(name)
	, m_uiSlotID(slotID)
	, m_ShaderType(shaderType)
	, m_ParameterType(parameterType)
{}

void ShaderParameter::SetConstantBuffer(std::shared_ptr<ConstantBuffer> buffer)
{
	m_pConstantBuffer = buffer;
}

void ShaderParameter::SetTexture(std::shared_ptr<pgTexture> texture)
{
	m_pTexture = texture;
}

void ShaderParameter::SetSampler(std::shared_ptr<SamplerState> sampler)
{
	m_pSamplerState = sampler;
}

void ShaderParameter::SetStructuredBuffer(std::shared_ptr<StructuredBuffer> rwBuffer)
{
	m_pStructuredBuffer = rwBuffer;
}

bool ShaderParameter::IsValid() const
{
	return m_ParameterType != ShaderParameter::Type::Invalid;
}

ShaderParameter::Type ShaderParameter::GetType() const
{
	return m_ParameterType;
}

static Shader::ShaderType getShaderType(const std::string& shaderType) {
	if (shaderType == "vs") {
		return Shader::ShaderType::VertexShader;
	}
	else if (shaderType == "ps") {
		return Shader::ShaderType::PixelShader;
	}
	else if (shaderType == "cs") {
		return Shader::ShaderType::ComputeShader;
	}
	else {
		assert(0);
	}

	return Shader::ShaderType::UnknownShaderType;
}

void ShaderParameter::Bind()
{
	Shader::ShaderType shaderType = getShaderType(m_ShaderType);

	if (std::shared_ptr<ConstantBuffer> constantBuffer = m_pConstantBuffer.lock())
	{
		constantBuffer->Bind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<pgTexture> texture = m_pTexture.lock())
	{
		texture->Bind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<SamplerState> samplerState = m_pSamplerState.lock())
	{
		samplerState->Bind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<StructuredBuffer> buffer = m_pStructuredBuffer.lock())
	{
		buffer->Bind(m_uiSlotID, shaderType, m_ParameterType);
	}
}

void ShaderParameter::UnBind()
{
	Shader::ShaderType shaderType = getShaderType(m_ShaderType);

	if (std::shared_ptr<ConstantBuffer> constantBuffer = m_pConstantBuffer.lock())
	{
		constantBuffer->UnBind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<pgTexture> texture = m_pTexture.lock())
	{
		texture->UnBind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<SamplerState> samplerState = m_pSamplerState.lock())
	{
		samplerState->UnBind(m_uiSlotID, shaderType, m_ParameterType);
	}
	if (std::shared_ptr<StructuredBuffer> buffer = m_pStructuredBuffer.lock())
	{
		buffer->UnBind(m_uiSlotID, shaderType, m_ParameterType);
	}
}

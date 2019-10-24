#include "../engine.h"

#include <limits>

namespace ade
{

ShaderParameter::ShaderParameter(const std::string& name, const std::string& shaderType,
                                 Type parameterType)
    : m_Name(name), m_uiSlotID((uint32_t)-1), m_ShaderType(shaderType),
      m_ParameterType(parameterType)
{
}

std::weak_ptr<Object> ShaderParameter::Get()
{
    return m_pResource;
}

void ShaderParameter::Set(std::shared_ptr<Object> resource)
{
    m_pResource = resource;
}

bool ShaderParameter::IsValid() const
{
    return m_ParameterType != ShaderParameter::Type::Invalid;
}

ShaderParameter::Type ShaderParameter::GetType() const
{
    return m_ParameterType;
}

const std::string& ShaderParameter::GetName() const
{
    return m_Name;
}


static Shader::ShaderType getShaderType(const std::string& shaderType)
{
    if (shaderType == "vs") {
        return Shader::ShaderType::VertexShader;
    } else if (shaderType == "ps") {
        return Shader::ShaderType::PixelShader;
    } else if (shaderType == "cs") {
        return Shader::ShaderType::ComputeShader;
    } else {
        assert(0);
    }

    return Shader::ShaderType::UnknownShaderType;
}

void ShaderParameter::Bind()
{
    Shader::ShaderType shaderType = getShaderType(m_ShaderType);

    if (std::shared_ptr<Object> pResource = m_pResource.lock()) {

        if (m_ParameterType == Type::CBuffer) {
            std::shared_ptr<ConstantBuffer> constantBuffer =
                std::dynamic_pointer_cast<ConstantBuffer>(pResource);
            constantBuffer->Bind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Texture || m_ParameterType == Type::RWTexture) {
            std::shared_ptr<Texture> texture = std::dynamic_pointer_cast<Texture>(pResource);

            texture->Bind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Sampler) {
            std::shared_ptr<SamplerState> samplerState =
                std::dynamic_pointer_cast<SamplerState>(pResource);

            samplerState->Bind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Buffer || m_ParameterType == Type::RWBuffer) {
            std::shared_ptr<Buffer> buffer = std::dynamic_pointer_cast<Buffer>(pResource);

            buffer->Bind(m_uiSlotID, shaderType, m_ParameterType);
        } else {
            CHECK_ERR(0);
        }
    }
}

void ShaderParameter::UnBind()
{
    Shader::ShaderType shaderType = getShaderType(m_ShaderType);

    if (std::shared_ptr<Object> pResource = m_pResource.lock()) {

        if (m_ParameterType == Type::CBuffer) {
            std::shared_ptr<ConstantBuffer> constantBuffer =
                std::dynamic_pointer_cast<ConstantBuffer>(pResource);
            constantBuffer->UnBind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Texture || m_ParameterType == Type::RWTexture) {
            std::shared_ptr<Texture> texture = std::dynamic_pointer_cast<Texture>(pResource);

            texture->UnBind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Sampler) {
            std::shared_ptr<SamplerState> samplerState =
                std::dynamic_pointer_cast<SamplerState>(pResource);

            samplerState->UnBind(m_uiSlotID, shaderType, m_ParameterType);
        } else if (m_ParameterType == Type::Buffer || m_ParameterType == Type::RWBuffer) {
            std::shared_ptr<Buffer> buffer = std::dynamic_pointer_cast<Buffer>(pResource);

            buffer->UnBind(m_uiSlotID, shaderType, m_ParameterType);
        } else {
            CHECK_ERR(0);
        }
    }
}

}    // namespace ade
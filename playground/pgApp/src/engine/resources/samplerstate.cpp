#include "../engine.h"

SamplerState::SamplerState(const Diligent::StaticSamplerDesc& desc) : base()
{
    Set(desc);
}


SamplerState::~SamplerState() {}


void SamplerState::Set(const Diligent::StaticSamplerDesc& desc)
{
    m_desc = desc;
}

const Diligent::StaticSamplerDesc& SamplerState::Get() const
{
    return m_desc;
}

// Bind this sampler state to the ID for a specific shader type.
void SamplerState::Bind(uint32_t ID, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType)
{
}

// Unbind the sampler state.
void SamplerState::UnBind(uint32_t ID, Shader::ShaderType shaderType,
                          ShaderParameter::Type parameterType)
{
}

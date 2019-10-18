#include "../engine.h"

Shader::Shader()
    : m_ShaderType( UnknownShaderType )
{
}

Shader::~Shader()
{
    Destroy();
}

void Shader::Destroy()
{
    m_pShader.Release();

    m_ShaderParameters.clear();
    m_InputSemantics.clear();
}

Shader::ShaderType Shader::GetType() const
{
    return m_ShaderType;
}

bool Shader::LoadShaderFromFile( ShaderType shaderType, 
	const std::string& fileName, const std::string& searchPaths,
	const std::string& entryPoint, const ShaderMacros& shaderMacros)
{
	Diligent::ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
	pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory(searchPaths.c_str(), &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

	m_ShaderType = shaderType;

	std::string shaderTypeStr = "";

	if (shaderType == ShaderType::VertexShader)	{
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pShader);
		shaderTypeStr = "vs";
	} else if (shaderType == ShaderType::PixelShader) {
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pShader);
		shaderTypeStr = "ps";
	} else if (shaderType == ShaderType::ComputeShader) {
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pShader);
		shaderTypeStr = "cs";
	}
	else {
		assert(0);
	}

	if (m_pShader) {
		LOG_INFO_MESSAGE("===============loading ", fileName, " ", shaderTypeStr);
		auto resCount = m_pShader->GetResourceCount();
		for (uint32_t i = 0; i < resCount; ++i) {
			auto res = m_pShader->GetResource(i);

			std::string resourceName = res.Name;

			ShaderParameter::Type parameterType = ShaderParameter::Type::Invalid;

			switch (res.Type)
			{
			case Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV:
				parameterType = ShaderParameter::Type::Texture;
				break;
			case Diligent::SHADER_RESOURCE_TYPE_SAMPLER:
				parameterType = ShaderParameter::Type::Sampler;
				break;
			case Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER:
				parameterType = ShaderParameter::Type::CBuffer;
				break;
			case Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV:
				parameterType = ShaderParameter::Type::Buffer;
				break;
			case Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV:
				parameterType = ShaderParameter::Type::RWBuffer;
				break;
			case Diligent::SHADER_RESOURCE_TYPE_TEXTURE_UAV:
				parameterType = ShaderParameter::Type::RWTexture;
				break;
			}

			// Create an empty shader parameter that should be filled-in by the application.
			std::shared_ptr<ShaderParameter> shaderParameter = std::make_shared<ShaderParameter>(resourceName, shaderTypeStr, parameterType);
			m_ShaderParameters.insert(ParameterMap::value_type(resourceName, shaderParameter));

			LOG_INFO_MESSAGE("===============ShaderParamter:", resourceName.c_str());
		}
	}

	return true;
}

ShaderParameter& Shader::GetShaderParameterByName( const std::string& name ) const
{
    ParameterMap::const_iterator iter = m_ShaderParameters.find( name );
    if ( iter != m_ShaderParameters.end() )
    {
        return *( iter->second );
    }

	CHECK_ERR(0, name.c_str(), " does not exist");

	static ShaderParameter gs_InvalidShaderParameter("invalid", "vs", ShaderParameter::Type::Invalid);

    return gs_InvalidShaderParameter;
}


bool Shader::HasSemantic( const pgBufferBinding& binding ) const
{
    SemanticMap::const_iterator iter = m_InputSemantics.find( binding );
    return iter != m_InputSemantics.end();
}

uint32_t Shader::GetSlotIDBySemantic( const pgBufferBinding& binding ) const
{
    SemanticMap::const_iterator iter = m_InputSemantics.find( binding );
    if ( iter != m_InputSemantics.end() )
    {
        return iter->second;
    }

    // Some kind of error code or exception...
    return (uint32_t)-1;
}

Shader::ParametersList Shader::GetConstantBuffers()
{
	Shader::ParametersList ret;

	for (ParameterMap::value_type value : m_ShaderParameters)
	{
		auto p = value.second;
		if (p->GetType() == ShaderParameter::Type::CBuffer)
		{ 
			ret.push_back(p);
		}
	}

	return ret;
}


void Shader::Bind()
{
    for ( ParameterMap::value_type value : m_ShaderParameters )
    {
        value.second->Bind();
    }

}

void Shader::UnBind()
{
    for ( ParameterMap::value_type value : m_ShaderParameters )
    {
        value.second->UnBind();
    }

}

void Shader::Dispatch( const Diligent::uint3& numGroups )
{
}



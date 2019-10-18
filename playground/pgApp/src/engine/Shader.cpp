#include "engine.h"

// This parameter will be returned if an invalid shader parameter is requested.
static ShaderParameter gs_InvalidShaderParameter;

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
    m_pPixelShader.Release();
    m_pDomainShader.Release();
    m_pHullShader.Release();
    m_pGeometryShader.Release();
    m_pVertexShader.Release();
    m_pComputeShader.Release();

    m_ShaderParameters.clear();
    m_InputSemantics.clear();
}

Shader::ShaderType Shader::GetType() const
{
    return m_ShaderType;
}


std::string Shader::GetLatestProfile( ShaderType type )
{
    // Throw an exception?
    return "";
}

bool Shader::LoadShaderFromFile( ShaderType shaderType, 
	const std::string& fileName, 
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
	pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

	if (shaderType == ShaderType::VertexShader)	{
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pVertexShader);
	} else if (shaderType == ShaderType::PixelShader) {
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pPixelShader);
	} else if (shaderType == ShaderType::ComputeShader) {
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = entryPoint.c_str();
		ShaderCI.Desc.Name = entryPoint.c_str();
		ShaderCI.FilePath = fileName.c_str();
		pgApp::s_device->CreateShader(ShaderCI, &m_pComputeShader);
	}
	else {
		assert(0);
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



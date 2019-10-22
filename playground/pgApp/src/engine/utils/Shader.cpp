#include "../engine.h"

Shader::Shader() : m_ShaderType(UnknownShaderType) {}

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

bool Shader::LoadShaderFromFile(ShaderType shaderType, const std::string& fileName,
                                const std::string& entryPoint, const std::string& searchPaths,
                                bool UseCombinedTextureSamplers,
                                const Diligent::ShaderMacro* shaderMacros)
{
    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture +
    // g_Texture_sampler combination)
    ShaderCI.UseCombinedTextureSamplers = UseCombinedTextureSamplers;

    // Create a shader source stream factory to load shaders from files.
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory(searchPaths.c_str(),
                                                                   &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

	if (shaderMacros) {
	    ShaderCI.Macros = shaderMacros;
	}

    m_ShaderType = shaderType;

    std::string shaderTypeStr = "";

    if (shaderType == ShaderType::VertexShader) {
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
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE;
        ShaderCI.EntryPoint = entryPoint.c_str();
        ShaderCI.Desc.Name = entryPoint.c_str();
        ShaderCI.FilePath = fileName.c_str();
        pgApp::s_device->CreateShader(ShaderCI, &m_pShader);
        shaderTypeStr = "cs";
    } else {
        assert(0);
    }

    m_InputSemantics.clear();

    // Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
    // hr = D3DReflect(m_pShaderBlob->GetBufferPointer(), m_pShaderBlob->GetBufferSize(),
    //                IID_ID3D11ShaderReflection, &pReflector);
    // D3D11_SHADER_DESC shaderDescription;
    // hr = pReflector->GetDesc(&shaderDescription);
    // UINT numInputParameters = shaderDescription.InputParameters;
    // std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    // for (UINT i = 0; i < numInputParameters; ++i) {
    //    D3D11_INPUT_ELEMENT_DESC inputElement;
    //    D3D11_SIGNATURE_PARAMETER_DESC parameterSignature;

    //    pReflector->GetInputParameterDesc(i, &parameterSignature);

    //    inputElement.SemanticName = parameterSignature.SemanticName;
    //    inputElement.SemanticIndex = parameterSignature.SemanticIndex;
    //    inputElement.InputSlot =
    //        i;    // TODO: If using interleaved arrays, then the input slot should be 0.  If using
    //              // packed arrays, the input slot will vary.
    //    inputElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    //    inputElement.InputSlotClass =
    //        D3D11_INPUT_PER_VERTEX_DATA;    // TODO: Figure out how to deal with per-instance
    //        data?
    //                                        // .. Don't. Just use structured buffers to store
    //                                        // per-instance data and use the SV_InstanceID as an
    //                                        // index in the structured buffer.
    //    inputElement.InstanceDataStepRate = 0;
    //    inputElement.Format = GetDXGIFormat(parameterSignature);

    //    assert(inputElement.Format != DXGI_FORMAT_UNKNOWN);

    //    inputElements.push_back(inputElement);

    //    m_InputSemantics.insert(SemanticMap::value_type(
    //        BufferBinding(inputElement.SemanticName, inputElement.SemanticIndex), i));
    //}

    if (m_pShader) {
        LOG_INFO_MESSAGE("===============loading ", fileName, " ", shaderTypeStr);
        auto resCount = m_pShader->GetResourceCount();
        for (uint32_t i = 0; i < resCount; ++i) {
            auto res = m_pShader->GetResource(i);

            std::string resourceName = res.Name;

            ShaderParameter::Type parameterType = ShaderParameter::Type::Invalid;

            switch (res.Type) {
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
            std::shared_ptr<ShaderParameter> shaderParameter =
                std::make_shared<ShaderParameter>(resourceName, shaderTypeStr, parameterType);
            m_ShaderParameters.insert(ParameterMap::value_type(resourceName, shaderParameter));

            LOG_INFO_MESSAGE("===============>>>>ShaderParamter:", resourceName.c_str());
        }
    }

    return true;
}

ShaderParameter& Shader::GetShaderParameterByName(const std::string& name) const
{
    ParameterMap::const_iterator iter = m_ShaderParameters.find(name);
    if (iter != m_ShaderParameters.end()) {
        return *(iter->second);
    }

    CHECK_ERR(0, name.c_str(), " does not exist");

    static ShaderParameter gs_InvalidShaderParameter("invalid", "vs",
                                                     ShaderParameter::Type::Invalid);

    return gs_InvalidShaderParameter;
}


bool Shader::HasSemantic(const pgBufferBinding& binding) const
{
    SemanticMap::const_iterator iter = m_InputSemantics.find(binding);
    return iter != m_InputSemantics.end();
}

uint32_t Shader::GetSlotIDBySemantic(const pgBufferBinding& binding) const
{
    SemanticMap::const_iterator iter = m_InputSemantics.find(binding);
    if (iter != m_InputSemantics.end()) {
        return iter->second;
    }

    // Some kind of error code or exception...
    return (uint32_t)-1;
}

static bool isStaticType(ShaderParameter::Type type)
{
    return (type == ShaderParameter::Type::CBuffer || type == ShaderParameter::Type::Buffer ||
            type == ShaderParameter::Type::RWBuffer || type == ShaderParameter::Type::Texture ||
            type == ShaderParameter::Type::RWTexture);
}

Shader::ParametersList Shader::GetConstantBuffers()
{
    Shader::ParametersList ret;

    for (ParameterMap::value_type value : m_ShaderParameters) {
        auto p = value.second;
        if (isStaticType(p->GetType())) {
            ret.push_back(p);
        }
    }

    return ret;
}


Shader::ParametersList Shader::GetNonConstantBuffers()
{
    Shader::ParametersList ret;

    for (ParameterMap::value_type value : m_ShaderParameters) {
        auto p = value.second;
        if (!isStaticType(p->GetType())) {
            ret.push_back(p);
        }
    }

    return ret;
}

void Shader::Bind()
{
    for (ParameterMap::value_type value : m_ShaderParameters) {
        value.second->Bind();
    }
}

void Shader::UnBind()
{
    for (ParameterMap::value_type value : m_ShaderParameters) {
        value.second->UnBind();
    }
}

#include "../engine.h"

pgPipeline::pgPipeline(std::shared_ptr<pgRenderTarget> rt)
	: m_pRT(rt), m_bDirty(true)
{
	m_PSODesc.Name = "pgPipeline PSO";

	// This is a graphics pipeline
	m_PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	m_PSODesc.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	m_PSODesc.GraphicsPipeline.RTVFormats[0] = pgApp::s_desc.ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	m_PSODesc.GraphicsPipeline.DSVFormat = pgApp::s_desc.DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	m_PSODesc.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// Cull back faces
	m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
	// Enable depth testing
	m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::True;

	// Define variable type that will be used by default
	m_PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
}

pgPipeline::~pgPipeline() {
	delete[] m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements;
}

void pgPipeline::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	if (flag & pgBindFlag::pgBindFlag_Pass) {
		if (m_pRT) {
			m_pRT->bind();
		}
	}

	e.pApp->bind(e, pgBindFlag::pgBindFlag_Pass);

	if (flag & pgBindFlag::pgBindFlag_Mesh) {
		// Set the pipeline state
		pgApp::s_ctx->SetPipelineState(m_pPSO);

		// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
		// makes sure that resources are transitioned to required states.
		pgApp::s_ctx->CommitShaderResources(m_pSRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
}

void pgPipeline::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}


void pgPipeline::SetLayoutElement(uint32_t layoutElementCount, const Diligent::LayoutElement* pLayoutElements) {
	m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = new Diligent::LayoutElement[layoutElementCount];
	memcpy((void*)m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements, pLayoutElements,
		sizeof(Diligent::LayoutElement) * layoutElementCount);
	m_PSODesc.GraphicsPipeline.InputLayout.NumElements = layoutElementCount;
}


void pgPipeline::SetShader( Shader::ShaderType type, std::shared_ptr<Shader> pShader )
{
    m_Shaders[type] = pShader;

	auto shader = pShader->GetShader();

	if (type == Shader::ShaderType::VertexShader) {
		m_PSODesc.GraphicsPipeline.pVS = shader;
	} else if (type == Shader::ShaderType::PixelShader) {
		m_PSODesc.GraphicsPipeline.pPS = shader;
	} else if (type == Shader::ShaderType::ComputeShader) {
		m_PSODesc.ComputePipeline.pCS = shader;
	} else {
		assert(0);
	}

	m_bDirty = true;
}

std::shared_ptr<Shader> pgPipeline::GetShader( Shader::ShaderType type ) const
{
    ShaderMap::const_iterator iter = m_Shaders.find( type );
    if ( iter != m_Shaders.end() )
    {
        return iter->second;
    }

    return nullptr;
}

const pgPipeline::ShaderMap& pgPipeline::GetShaders() const
{
    return m_Shaders;
}

void pgPipeline::SetBlendState( const Diligent::BlendStateDesc& blendState )
{
	m_PSODesc.GraphicsPipeline.BlendDesc =  blendState;
	m_bDirty = true;
}

Diligent::BlendStateDesc& pgPipeline::GetBlendState()
{
    return m_PSODesc.GraphicsPipeline.BlendDesc;
}

void pgPipeline::SetRasterizerState( const Diligent::RasterizerStateDesc& rasterizerState )
{
	m_PSODesc.GraphicsPipeline.RasterizerDesc = rasterizerState;
	m_bDirty = true;
}

Diligent::RasterizerStateDesc& pgPipeline::GetRasterizerState() 
{
    return m_PSODesc.GraphicsPipeline.RasterizerDesc;
}

void pgPipeline::SetDepthStencilState( const Diligent::DepthStencilStateDesc& depthStencilState )
{
	m_PSODesc.GraphicsPipeline.DepthStencilDesc = depthStencilState;
	m_bDirty = true;
}

Diligent::DepthStencilStateDesc& pgPipeline::GetDepthStencilState()
{
    return m_PSODesc.GraphicsPipeline.DepthStencilDesc;
}

void pgPipeline::SetRenderTarget( std::shared_ptr<pgRenderTarget> renderTarget )
{
    m_pRT = renderTarget;
	m_bDirty = true;
}

std::shared_ptr<pgRenderTarget> pgPipeline::GetRenderTarget() const
{
    return m_pRT;
}

void pgPipeline::Bind()
{
	if (m_bDirty) {
		pgApp::s_device->CreatePipelineState(m_PSODesc, &m_pPSO);

		// Create a shader resource binding object and bind all static resources in it
		m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

		m_bDirty = false;
	}

    if (m_pRT)
    {
		m_pRT->Bind();
    }

	// Set the pipeline state
	pgApp::s_ctx->SetPipelineState(m_pPSO);

    for ( auto shader : m_Shaders )
    {
        std::shared_ptr<Shader> pShader = shader.second;
        if ( pShader )
        {
            pShader->Bind();
        }
    }

	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
	// makes sure that resources are transitioned to required states.
	pgApp::s_ctx->CommitShaderResources(m_pSRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void pgPipeline::UnBind()
{
    if (m_pRT)
    {
		m_pRT->UnBind();
    }

    for ( auto shader : m_Shaders )
    {
        std::shared_ptr<Shader> pShader = shader.second;
        if ( pShader )
        {
            pShader->UnBind();
        }
    }
}
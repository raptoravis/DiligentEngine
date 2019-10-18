#include "engine.h"

pgPipeline::pgPipeline(std::shared_ptr<pgRenderTarget> rt)
	: m_pRT(rt)
{
	//
}

pgPipeline::~pgPipeline() {
	//
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



void pgPipeline::SetShader( Shader::ShaderType type, std::shared_ptr<Shader> pShader )
{
    m_Shaders[type] = pShader;
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
    m_BlendState =  blendState;
}

Diligent::BlendStateDesc& pgPipeline::GetBlendState()
{
    return m_BlendState;
}

void pgPipeline::SetRasterizerState( const Diligent::RasterizerStateDesc& rasterizerState )
{
    m_RasterizerState = rasterizerState;
}

Diligent::RasterizerStateDesc& pgPipeline::GetRasterizerState() 
{
    return m_RasterizerState;
}

void pgPipeline::SetDepthStencilState( const Diligent::DepthStencilStateDesc& depthStencilState )
{
    m_DepthStencilState = depthStencilState;
}

Diligent::DepthStencilStateDesc& pgPipeline::GetDepthStencilState()
{
    return m_DepthStencilState;
}

void pgPipeline::SetRenderTarget( std::shared_ptr<pgRenderTarget> renderTarget )
{
    m_pRT = renderTarget;
}

std::shared_ptr<pgRenderTarget> pgPipeline::GetRenderTarget() const
{
    return m_pRT;
}

void pgPipeline::Bind()
{
  //  if (m_pRT)
  //  {
		//m_pRT->Bind();
  //  }

  //  m_BlendState.Bind();
  //  m_RasterizerState.Bind();
  //  m_DepthStencilState.Bind();

  //  for ( auto shader : m_Shaders )
  //  {
  //      std::shared_ptr<Shader> pShader = shader.second;
  //      if ( pShader )
  //      {
  //          pShader->Bind();
  //      }
  //  }
}

void pgPipeline::UnBind()
{
  //  if (m_pRT)
  //  {
		//m_pRT->UnBind();
  //  }

  //  for ( auto shader : m_Shaders )
  //  {
  //      std::shared_ptr<Shader> pShader = shader.second;
  //      if ( pShader )
  //      {
  //          pShader->UnBind();
  //      }
  //  }
}
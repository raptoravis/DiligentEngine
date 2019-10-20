#include "pipelinecolorvertex.h"

using namespace Diligent;

PipelineColorVertex::PipelineColorVertex(std::shared_ptr<pgRenderTarget> rt) 
	: base(rt)
{
	//CreatePipelineState();
	m_pVS = std::make_shared<Shader>();
	m_pVS->LoadShaderFromFile(Shader::Shader::VertexShader, "cube.vsh", "main", "", true);

	m_pPS = std::make_shared<Shader>();
    m_pPS->LoadShaderFromFile(Shader::Shader::PixelShader, "cube.psh", "main", "", true);

	SetShader(Shader::Shader::VertexShader, m_pVS);
	SetShader(Shader::Shader::PixelShader, m_pPS);

	static LayoutElement LayoutElems[] =
	{
		// Attribute 0 - vertex position
		LayoutElement{0, 0, 3, VT_FLOAT32, False},
		// Attribute 1 - vertex color
		LayoutElement{1, 0, 4, VT_FLOAT32, False}
	};

	m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
	m_PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);
}

PipelineColorVertex::~PipelineColorVertex() {

}

void PipelineColorVertex::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	{
		if (flag == pgBindFlag::pgBindFlag_Mesh) {
			const float4x4 view = e.pCamera->getViewMatrix();

			const float4x4 local = e.pSceneNode->getLocalTransform();

			// Set cube world view matrix
			float4x4 CubeWorldView = float4x4::RotationY(static_cast<float>(e.CurrTime) * -1.0f) * local * view;

			auto& Proj = e.pCamera->getProjectionMatrix();

			// Compute world-view-projection matrix
			float4x4 worldViewProjMatrix = CubeWorldView * Proj;

			//{
			//	// Map the buffer and write current world-view-projection matrix
			//	MapHelper<float4x4> CBConstants(pgApp::s_ctx, m_VSConstants->GetBuffer(), MAP_WRITE, MAP_FLAG_DISCARD);
			//	*CBConstants = worldViewProjMatrix.Transpose();
			//}

			// Set the pipeline state
			pgApp::s_ctx->SetPipelineState(m_pPSO);

			// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
			// makes sure that resources are transitioned to required states.
			pgApp::s_ctx->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}
}


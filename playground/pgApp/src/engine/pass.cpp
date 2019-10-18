#include "engine.h"

AbstractPass::AbstractPass()
	: m_Enabled(true)
{
}

AbstractPass::~AbstractPass()
{}

// Enable or disable the pass. If a pass is disabled, the technique will skip it.
void AbstractPass::SetEnabled(bool enabled)
{
	m_Enabled = enabled;
}

bool AbstractPass::IsEnabled() const
{
	return m_Enabled;
}

// Render the pass. This should only be called by the RenderTechnique.
void AbstractPass::PreRender()
{}

void AbstractPass::Render()
{}

void AbstractPass::PostRender()
{}

// Inherited from Visitor
void AbstractPass::Visit(pgScene& scene)
{}

void AbstractPass::Visit(pgSceneNode& node)
{}

void AbstractPass::Visit(pgMesh& mesh)
{}


BasePass::BasePass()
{
	m_PerObjectData = (PerObject*)_aligned_malloc(sizeof(PerObject), 16);
}

BasePass::BasePass(std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline)
	: m_Scene(scene)
	, m_Pipeline(pipeline)
{
	m_PerObjectData = (PerObject*)_aligned_malloc(sizeof(PerObject), 16);
}

BasePass::~BasePass()
{
	_aligned_free(m_PerObjectData);
}

void BasePass::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
	m_PerObjectConstantBuffer->Set(perObjectData);
}

void BasePass::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
	if (shader)
	{
		shader->GetShaderParameterByName("PerObject").SetConstantBuffer(m_PerObjectConstantBuffer);
	}
}

void BasePass::PreRender()
{
	pgApp::s_eventArgs.pPipeline = m_Pipeline.get();

	if (m_Pipeline)
	{
		// Make sure the per object constant buffer is bound to the vertex shader.
		BindPerObjectConstantBuffer(m_Pipeline->GetShader(Shader::VertexShader));
		m_Pipeline->Bind();
	}
}

void BasePass::Render()
{
	if (m_Scene)
	{
		m_Scene->Accept(*this);
	}
}

void BasePass::PostRender()
{
	if (m_Pipeline)
	{
		m_Pipeline->UnBind();
	}
}

// Inherited from Visitor
void BasePass::Visit(pgScene& scene)
{

}

void BasePass::Visit(pgSceneNode& node)
{
	pgCamera* camera = pgApp::s_eventArgs.pCamera;
	if (camera)
	{
		PerObject perObjectData;
		// Update the constant buffer data for the node.
		Diligent::float4x4 viewMatrix = camera->getViewMatrix();
		perObjectData.ModelView = node.getWorldTransfom() * viewMatrix;
		perObjectData.ModelViewProjection = perObjectData.ModelView * camera->getProjectionMatrix();

		// Update the constant buffer data
		SetPerObjectConstantBufferData(perObjectData);
	}
}

void BasePass::Visit(pgMesh& mesh)
{
	std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
	if (pMaterial)
	{
		mesh.Render();
	}
}


OpaquePass::OpaquePass(std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline)
	: base(scene, pipeline)
{
}

OpaquePass::~OpaquePass()
{}

void OpaquePass::Visit(pgMesh& mesh)
{
	std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
	if (pMaterial && !pMaterial->IsTransparent())
	{
		mesh.Render();
	}
}

#include "../engine.h"

using namespace Diligent;

pgMesh::pgMesh() {
	//
}

pgMesh::~pgMesh() {
	//
}

void pgMesh::addVertexBuffer(const pgBufferBinding& binding, std::shared_ptr<pgBuffer> pgBuffer)
{
	m_VertexBuffers[binding] = pgBuffer;
}

void pgMesh::setIndexBuffer(std::shared_ptr<pgBuffer> pgBuffer)
{
	m_pIndexBuffer = pgBuffer;
}

void pgMesh::setMaterial(std::shared_ptr<pgMaterial> pgMaterial)
{
	m_pMaterial = pgMaterial;
}

std::shared_ptr<pgMaterial> pgMesh::getMaterial() const
{
	return m_pMaterial;
}

static uint32_t getSlot(const pgBufferBinding& binding) {
	if (binding.Name == "POSITION") {
		return 0;
	}
	else if (binding.Name == "VERTEX") {
		return 0;
	}
	else if (binding.Name == "TANGENT") {
		return 1;
	}
	else if (binding.Name == "BINORMAL") {
		return 2;
	}
	else if (binding.Name == "NORMAL") {
		return 3;
	}
	else if (binding.Name == "TEXCOORD") {
		return 4;
	}
	else {
		assert(0);
	}

	return 0;
}

void pgMesh::_render(pgRenderEventArgs& e) {
	auto oldMesh = e.pMesh;
	auto currentMesh = this;

	e.pMesh = currentMesh;
	currentMesh->bind(e, pgBindFlag::pgBindFlag_Mesh);

	currentMesh->render(e);

	// clear it
	currentMesh->unbind(e, pgBindFlag::pgBindFlag_Mesh);
	e.pMesh = oldMesh;
}

void pgMesh::render(pgRenderEventArgs& e)
{
	assert(e.pPass);

	//constexpr uint32_t kMaxBuffers = Diligent::MaxBufferSlots;

	Uint32 offset[] = {0};
	IBuffer *pBuffs[] = {0};
	const uint32_t buffs = 1;
	
	for (BufferMap::value_type buffer : m_VertexBuffers)
	{
		pgBufferBinding binding = buffer.first;
		uint32_t slot = getSlot(binding);

		pBuffs[0] = buffer.second->GetBuffer();

		pgApp::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_NONE);
	}

	pgApp::s_ctx->SetIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	auto count = m_pIndexBuffer->getCount();

	DrawIndexedAttribs DrawAttrs;
	DrawAttrs.IndexType = VT_UINT32; // Index type
	DrawAttrs.NumIndices = count;
	// Verify the state of vertex and index buffers
	DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

	pgApp::s_ctx->DrawIndexed(DrawAttrs);
}

void pgMesh::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	e.pPass->bind(e, flag);
}

void pgMesh::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	e.pPass->unbind(e, flag);
}

void pgMesh::Render() {
	std::shared_ptr<Shader> pVS;

	// Clone this mesh's material in case we want to override the 
	// shaders in the mesh's default material.
	//Material material( *m_pMaterial );

	// Use the vertex shader to convert the buffer semantics to slot ID's
	auto pipeline = pgApp::s_eventArgs.pPipeline;
	if (pipeline)
	{
		pVS = pipeline->GetShader(Shader::VertexShader);

		if (pVS)
		{
			for (BufferMap::value_type buffer : m_VertexBuffers)
			{
				pgBufferBinding binding = buffer.first;
				//if (pVS->HasSemantic(binding))
				//{
				//	uint32_t slotID = pVS->GetSlotIDBySemantic(binding);
				//	// Bind the vertex buffer to a particular slot ID.
				//	buffer.second->Bind(slotID, Shader::VertexShader, ShaderParameter::Type::Buffer);
				//}
                uint32_t slot = getSlot(binding);

				buffer.second->Bind(slot, Shader::VertexShader, ShaderParameter::Type::Buffer);
			}

			pgApp::s_ctx->SetIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}

		if (m_pMaterial)
		{
			for (auto shader : pipeline->GetShaders())
			{
				m_pMaterial->Bind(shader.second);
			}
		}

		auto count = m_pIndexBuffer->getCount();

		DrawIndexedAttribs DrawAttrs;
		DrawAttrs.IndexType = VT_UINT32; // Index type
		DrawAttrs.NumIndices = count;
		// Verify the state of vertex and index buffers
		DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

		pgApp::s_ctx->DrawIndexed(DrawAttrs);
	}
}

void pgMesh::Accept(Visitor& visitor) {
	visitor.Visit(*this);
}
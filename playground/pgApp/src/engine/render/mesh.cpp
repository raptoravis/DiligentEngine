#include "../engine.h"

using namespace Diligent;

void pgMesh::AddVertexBuffer(const pgBufferBinding& binding, std::shared_ptr<pgBuffer> pgBuffer)
{
	m_VertexBuffers[binding] = pgBuffer;
}

void pgMesh::SetIndexBuffer(std::shared_ptr<pgBuffer> pgBuffer)
{
	m_pIndexBuffer = pgBuffer;
}

void pgMesh::SetMaterial(std::shared_ptr<pgMaterial> pgMaterial)
{
	m_pMaterial = pgMaterial;
}

std::shared_ptr<pgMaterial> pgMesh::GetMaterial() const
{
	return m_pMaterial;
}

void pgMesh::render(pgSceneNode* sceneNode, pgRenderEventArgs& e)
{
	constexpr uint32_t kMaxBuffers = Diligent::MaxBufferSlots;

	Uint32 offset[kMaxBuffers] = {0};
	IBuffer *pBuffs[kMaxBuffers] = {0};
	uint32_t buffs = 0;

	for (BufferMap::value_type buffer : m_VertexBuffers)
	{
		pgBufferBinding binding = buffer.first;

		pBuffs[buffs++] = buffer.second->m_pBuffer.RawPtr();

		assert(buffs <= kMaxBuffers);
	}

	m_pImmediateContext->SetVertexBuffers(0, buffs, pBuffs, offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
	m_pImmediateContext->SetIndexBuffer(m_pIndexBuffer->m_pBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	e.pPass->updateSRB(sceneNode, e);

	if (m_pMaterial)
	{
	}

	auto count = m_pIndexBuffer->getCount();

	DrawAttribs DrawAttrs;
	DrawAttrs.IsIndexed = true;      // This is an indexed draw call
	DrawAttrs.IndexType = VT_UINT32; // Index type
	DrawAttrs.NumIndices = count;
	// Verify the state of vertex and index buffers
	DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

	m_pImmediateContext->Draw(DrawAttrs);
}

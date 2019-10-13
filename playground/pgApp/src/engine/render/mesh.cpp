#include "../engine.h"

using namespace Diligent;

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

		pBuffs[0] = buffer.second->m_pBuffer.RawPtr();

		m_pImmediateContext->SetVertexBuffers(slot, buffs, pBuffs, offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
	}

	m_pImmediateContext->SetIndexBuffer(m_pIndexBuffer->m_pBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	e.pPass->updateSRB(e);

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

#include "../engine.h"

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

void pgMesh::Render(pgRenderEventArgs& renderArgs)
{
}

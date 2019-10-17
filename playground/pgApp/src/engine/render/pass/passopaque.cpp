#include "passopaque.h"

PassOpaque::PassOpaque(const PassOpaqueCreateInfo& ci)
	: base(ci)
{
	PipelineStateDesc PSODesc;

	CreatePipelineState(PSODesc);
}

PassOpaque::~PassOpaque()
{
}

bool PassOpaque::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return !bTransparent;
}

// Render a frame
void PassOpaque::render(pgRenderEventArgs& e) {
	m_scene->render(e);
}

void PassOpaque::update(pgRenderEventArgs& e) {
	//
}

void PassOpaque::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::bind(e, flag);
}

void PassOpaque::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::unbind(e, flag);
}
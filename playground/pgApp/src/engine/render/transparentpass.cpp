#pragma once

#include "TransparentPass.h"

TransparentPass::TransparentPass(const BasePassCreateInfo& ci)
	: base(ci)
{
	CreatePipelineState(ci);
}

TransparentPass::~TransparentPass()
{
}

bool TransparentPass::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return bTransparent;
}

// Render a frame
void TransparentPass::render(pgRenderEventArgs& e) {
	m_scene->render(e);
}

void TransparentPass::update(pgRenderEventArgs& e) {
	//
}

void TransparentPass::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
	base::updateSRB(e, flag);
}
#pragma once

#include "OpaquePass.h"

OpaquePass::OpaquePass(const RenderPassCreateInfo& ci)
	: base(ci)
{
	CreatePipelineState(ci);
}

OpaquePass::~OpaquePass()
{
}

bool OpaquePass::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return !bTransparent;
}

// Render a frame
void OpaquePass::render(pgRenderEventArgs& e) {
	m_scene->render(e);
}

void OpaquePass::update(pgRenderEventArgs& e) {
	//
}

void OpaquePass::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
	base::updateSRB(e, flag);
}
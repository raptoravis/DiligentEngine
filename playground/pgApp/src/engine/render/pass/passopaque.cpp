#pragma once

#include "passopaque.h"

OpaquePass::OpaquePass(const OpaquePassCreateInfo& ci)
	: base(ci)
{
	if (!m_pPipeline) {
		PipelineStateDesc PSODesc;

		CreatePipelineState(ci, PSODesc);
	}
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
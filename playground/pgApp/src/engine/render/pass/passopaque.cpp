#pragma once

#include "passopaque.h"

PassOpaque::PassOpaque(const OpaquePassCreateInfo& ci)
	: base(ci)
{
	PipelineStateDesc PSODesc;

	CreatePipelineState(ci, PSODesc);
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

void PassOpaque::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
	base::updateSRB(e, flag);
}
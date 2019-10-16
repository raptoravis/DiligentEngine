#pragma once

#include "passtransparent.h"

PassTransparent::PassTransparent(const pgPassRenderCreateInfo& ci)
	: base(ci)
{
	PipelineStateDesc PSODesc;

	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

	CreatePipelineState(ci, PSODesc);
}

PassTransparent::~PassTransparent()
{
}

bool PassTransparent::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return bTransparent;
}

// Render a frame
void PassTransparent::render(pgRenderEventArgs& e) {
	m_scene->render(e);
}

void PassTransparent::update(pgRenderEventArgs& e) {
	//
}

void PassTransparent::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
	base::updateSRB(e, flag);
}
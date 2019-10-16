#pragma once

#include "passtransparent.h"

TransparentPass::TransparentPass(const RenderPassCreateInfo& ci)
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
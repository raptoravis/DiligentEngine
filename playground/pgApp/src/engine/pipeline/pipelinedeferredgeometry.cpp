#include "pipelinedeferredgeometry.h"

using namespace Diligent;

namespace ade
{

PipelineDeferredGeometry::PipelineDeferredGeometry(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineDeferredGeometry::~PipelineDeferredGeometry() {}

void PipelineDeferredGeometry::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineDeferredGeometry PSO";

    // auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    // auto color0Format =
    //    color0 ? color0->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    // auto ds = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    // auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : pgApp::s_desc.DepthBufferFormat;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = 4;

    // difuse
    m_PSODesc.GraphicsPipeline.RTVFormats[1] = TEX_FORMAT_RGBA8_UNORM;
    // specular
    m_PSODesc.GraphicsPipeline.RTVFormats[2] = TEX_FORMAT_RGBA8_UNORM;
    // normal
    m_PSODesc.GraphicsPipeline.RTVFormats[3] = TEX_FORMAT_RGBA32_FLOAT;
}

}    // namespace ade
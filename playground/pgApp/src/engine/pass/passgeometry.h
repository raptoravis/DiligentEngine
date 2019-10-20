#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "../utils/light.h"
#include "passrender.h"

using namespace Diligent;

class PassGeometry : public pgPassRender
{
    typedef pgPassRender base;

  protected:
    // void CreatePipelineState(PipelineStateDesc& PSODesc);

    std::shared_ptr<pgRenderTarget> m_pGBufferRT;

  public:
    PassGeometry(pgTechnique* parentTechnique, const std::shared_ptr<pgScene> scene,
                 std::shared_ptr<pgPipeline> pipeline,
                 const std::vector<pgLight>& lights, std::shared_ptr<pgRenderTarget> rt);

    virtual ~PassGeometry();
};

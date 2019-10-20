#pragma once

#include "engine/engine.h"

class PipelineLightBack : public pgPipeline
{
    typedef pgPipeline base;

  private:
    void CreatePipelineState();
    std::shared_ptr<pgRenderTarget> m_pGBufferRT;

  public:
    PipelineLightBack(std::shared_ptr<pgRenderTarget> rt,
                      std::shared_ptr<pgRenderTarget> GBufferRT);

    virtual ~PipelineLightBack();
};
#pragma once

#include "engine/engine.h"

class PipelineLightDir : public pgPipeline
{
    typedef pgPipeline base;

  private:
    void CreatePipelineState();
    std::shared_ptr<pgRenderTarget> m_pGBufferRT;

  public:
    PipelineLightDir(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgRenderTarget> GBufferRT);

    virtual ~PipelineLightDir();

    virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
    virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);
};
#pragma once

#include "engine/engine.h"

class PipelineLightFront : public pgPipeline
{
    typedef pgPipeline base;

  private:
    void CreatePipelineState();

  public:
    PipelineLightFront(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineLightFront();
};
#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineLightDir : public PipelineBase
{
    typedef PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineLightDir(std::shared_ptr<pgRenderTarget> rt);

    virtual ~PipelineLightDir();
};
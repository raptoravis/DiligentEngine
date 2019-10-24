#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineDeferredGeometry : public PipelineBase
{
    typedef PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineDeferredGeometry(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineDeferredGeometry();
};
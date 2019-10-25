#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineDeferredGeometry : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineDeferredGeometry(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineDeferredGeometry();
};


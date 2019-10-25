#pragma once

#include "engine/pipeline/pipelinebase.h"


class PipelineLightFront : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineLightFront(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineLightFront();
};


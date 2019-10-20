#pragma once

#include "engine/pipeline//pipelinebase.h"

class PipelineLightBack : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineLightBack(std::shared_ptr<pgRenderTarget> rt);

    virtual ~PipelineLightBack();
};
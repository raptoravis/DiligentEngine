#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineTransparent : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTransparent(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineTransparent();
};
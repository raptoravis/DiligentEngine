#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineTransparent : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTransparent(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineTransparent();
};


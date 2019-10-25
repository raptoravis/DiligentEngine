#pragma once

#include "engine/pipeline/pipelinebase.h"


class PipelineGdr : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineGdr(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineGdr();
};


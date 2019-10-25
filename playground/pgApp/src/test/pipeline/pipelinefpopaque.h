#pragma once

#include "engine/pipeline/pipelinebase.h"


class PipelineFPOpaque : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineFPOpaque(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineFPOpaque();
};


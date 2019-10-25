#pragma once

#include "engine/pipeline//pipelinebase.h"


class PipelineLightBack : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineLightBack(std::shared_ptr<ade::RenderTarget> rt);

    virtual ~PipelineLightBack();
};


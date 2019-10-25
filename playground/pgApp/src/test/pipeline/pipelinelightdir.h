#pragma once

#include "engine/pipeline/pipelinebase.h"


class PipelineLightDir : public ade::PipelineBase
{
    typedef ade::PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineLightDir(std::shared_ptr<ade::RenderTarget> rt);

    virtual ~PipelineLightDir();
};


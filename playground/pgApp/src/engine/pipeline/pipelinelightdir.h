#pragma once

#include "engine/pipeline/pipelinebase.h"

namespace ade
{

class PipelineLightDir : public PipelineBase
{
    typedef PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineLightDir(std::shared_ptr<RenderTarget> rt);

    virtual ~PipelineLightDir();
};

}    // namespace ade
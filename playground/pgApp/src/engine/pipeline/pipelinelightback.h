#pragma once

#include "engine/pipeline//pipelinebase.h"

namespace ade
{

class PipelineLightBack : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineLightBack(std::shared_ptr<pgRenderTarget> rt);

    virtual ~PipelineLightBack();
};

}    // namespace ade
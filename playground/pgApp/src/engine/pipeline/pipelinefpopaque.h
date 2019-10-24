#pragma once

#include "engine/pipeline/pipelinebase.h"

namespace ade
{

class PipelineFPOpaque : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineFPOpaque(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineFPOpaque();
};

}    // namespace ade
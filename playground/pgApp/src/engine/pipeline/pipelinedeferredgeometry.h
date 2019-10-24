#pragma once

#include "engine/pipeline/pipelinebase.h"

namespace ade
{

class PipelineDeferredGeometry : public PipelineBase
{
    typedef PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineDeferredGeometry(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineDeferredGeometry();
};

}    // namespace ade
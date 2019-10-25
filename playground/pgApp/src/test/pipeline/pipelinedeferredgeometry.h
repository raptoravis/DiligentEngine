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
    PipelineDeferredGeometry(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineDeferredGeometry();
};

}    // namespace ade
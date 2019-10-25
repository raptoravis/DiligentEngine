#pragma once

#include "engine/pipeline/pipelinebase.h"

namespace ade
{

class PipelineTransparent : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTransparent(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineTransparent();
};

}    // namespace ade
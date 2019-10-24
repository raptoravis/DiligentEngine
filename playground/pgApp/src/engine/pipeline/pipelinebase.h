#pragma once

#include "engine/engine.h"

namespace ade
{

class PipelineBase : public Pipeline
{
    typedef Pipeline base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineBase(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineBase();
};

}    // namespace ade
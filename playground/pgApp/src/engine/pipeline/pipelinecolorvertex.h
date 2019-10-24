#pragma once

#include "engine/engine.h"

namespace ade
{

class PipelineColorVertex : public pgPipeline
{
    typedef pgPipeline base;

  private:
    std::shared_ptr<Shader> m_pVS;
    std::shared_ptr<Shader> m_pPS;

  public:
    PipelineColorVertex(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineColorVertex();
};

}    // namespace ade
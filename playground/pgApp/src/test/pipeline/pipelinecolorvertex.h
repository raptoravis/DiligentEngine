#pragma once

#include "engine/engine.h"

namespace ade
{

class PipelineColorVertex : public Pipeline
{
    typedef Pipeline base;

  private:
    std::shared_ptr<Shader> m_pVS;
    std::shared_ptr<Shader> m_pPS;

  public:
    PipelineColorVertex(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineColorVertex();
};

}    // namespace ade
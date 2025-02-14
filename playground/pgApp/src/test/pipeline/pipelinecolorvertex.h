#pragma once

#include "engine/engine.h"

class PipelineColorVertex : public ade::Pipeline
{
    typedef ade::Pipeline base;

  private:
    std::shared_ptr<ade::Shader> m_pVS;
    std::shared_ptr<ade::Shader> m_pPS;

  public:
    PipelineColorVertex(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineColorVertex();
};


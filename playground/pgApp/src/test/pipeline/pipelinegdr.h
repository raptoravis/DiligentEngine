#pragma once

#include "engine/engine.h"

class PipelineGdr : public ade::Pipeline
{
    typedef ade::Pipeline base;

  private:
    std::shared_ptr<ade::Shader> m_pVS;
    std::shared_ptr<ade::Shader> m_pPS;

  public:
    static const char* kPerObjectName;
    static const char* kColorsMaterialName;

    PipelineGdr(std::shared_ptr<ade::RenderTarget> rt, std::shared_ptr<ade::ConstantBuffer> perObjectCB,
                std::shared_ptr<ade::ConstantBuffer> colors);
    virtual ~PipelineGdr();
};

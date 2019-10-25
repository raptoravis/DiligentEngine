#pragma once

#include "engine/engine.h"

class PipelineTexVertex : public ade::Pipeline
{
    typedef ade::Pipeline base;

  private:
    std::shared_ptr<ade::Shader> m_pVS;
    std::shared_ptr<ade::Shader> m_pPS;

    std::shared_ptr<ade::Texture> m_Texture;
    std::shared_ptr<ade::SamplerState> m_LinearClampSampler;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTexVertex(std::shared_ptr<ade::RenderTarget> rt);
    virtual ~PipelineTexVertex();

    void LoadTexture();
};


#pragma once

#include "engine/engine.h"

class PipelineTexVertex : public pgPipeline
{
    typedef pgPipeline base;

  private:
    std::shared_ptr<Shader> m_pVS;
    std::shared_ptr<Shader> m_pPS;

    std::shared_ptr<pgTexture> m_Texture;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTexVertex(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineTexVertex();

    void LoadTexture();
};
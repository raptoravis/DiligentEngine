#pragma once

#include "engine/engine.h"

namespace ade
{

class PipelineTexVertex : public Pipeline
{
    typedef Pipeline base;

  private:
    std::shared_ptr<Shader> m_pVS;
    std::shared_ptr<Shader> m_pPS;

    std::shared_ptr<Texture> m_Texture;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineTexVertex(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineTexVertex();

    void LoadTexture();
};

}    // namespace ade
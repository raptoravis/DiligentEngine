#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "passrender.h"

namespace ade
{

class PassPostprocess : public pgPassRender
{
  public:
    typedef pgPassRender base;

    PassPostprocess(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                    std::shared_ptr<pgPipeline> pipeline,
                    const Diligent::float4x4& projectionMatrix, std::shared_ptr<pgTexture> texture);

    virtual void PreRender();
    virtual void Render(pgPipeline* pipeline);

    virtual void Visit(pgSceneNode& node, pgPipeline* pipeline);

  protected:
  private:
    Diligent::float4x4 m_ProjectionMatrix;
    std::shared_ptr<pgTexture> m_Texture;
};
}    // namespace ade
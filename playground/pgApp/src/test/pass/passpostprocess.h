#pragma once

#include "SampleBase.h"

#include "engine/engine.h"
#include "passrender.h"

using namespace ade;


class PassPostprocess : public PassRender
{
  public:
    typedef PassRender base;

    PassPostprocess(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                    std::shared_ptr<Pipeline> pipeline,
                    const Diligent::float4x4& projectionMatrix, std::shared_ptr<Texture> texture);

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);

    virtual void Visit(SceneNode& node, Pipeline* pipeline);

  protected:
  private:
    Diligent::float4x4 m_ProjectionMatrix;
    std::shared_ptr<Texture> m_Texture;
};

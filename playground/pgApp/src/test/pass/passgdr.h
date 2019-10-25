#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "test/pass/passrender.h"

using namespace Diligent;
using namespace ade;

class PassGdr : public PassRender
{
    typedef PassRender base;

  protected:
    // void CreatePipelineState(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture>
    // backBuffer);
  public:
    PassGdr(Technique* parentTechnique, std::shared_ptr<Scene> scene,
               std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights);

    virtual ~PassGdr();

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);

    virtual void Visit(Mesh& mesh, Pipeline* pipeline);
};

#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "../utils/light.h"
#include "passrender.h"

using namespace Diligent;

class PassOpaque : public pgPassRender
{
    typedef pgPassRender base;

  protected:
    // void CreatePipelineState(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture>
    // backBuffer);
  public:
    PassOpaque(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
               std::shared_ptr<pgPipeline> pipeline, const std::vector<pgLight>& lights);

    virtual ~PassOpaque();

    virtual void Visit(pgMesh& mesh);
};

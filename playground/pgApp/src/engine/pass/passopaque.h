#pragma once

#include "SampleBase.h"

#include "../engine.h"

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
               std::shared_ptr<pgPipeline> pipeline, std::vector<pgLight>* lights);

    virtual ~PassOpaque();

	virtual void PreRender();
    virtual void Render(pgPipeline* pipeline);

    virtual void Visit(pgMesh& mesh, pgPipeline* pipeline);
};

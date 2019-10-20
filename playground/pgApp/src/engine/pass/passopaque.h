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
    PassOpaque(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
               const std::vector<pgLight>& lights);

    virtual ~PassOpaque();

    // Render the pass. This should only be called by the pgTechnique.
    virtual void update(pgRenderEventArgs& e);
    virtual void render(pgRenderEventArgs& e);
    virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
    virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

    virtual bool meshFilter(pgMesh* mesh);

    virtual void Visit(pgMesh& mesh);
};

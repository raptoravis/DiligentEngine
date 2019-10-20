#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "../utils/light.h"
#include "passrender.h"

using namespace Diligent;

class PassGeometry : public pgPassRender
{
    typedef pgPassRender base;

  protected:
    // void CreatePipelineState(PipelineStateDesc& PSODesc);

    std::shared_ptr<pgRenderTarget> m_pGBufferRT;

  public:
    PassGeometry(const std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
                 const std::vector<pgLight>& lights, std::shared_ptr<pgRenderTarget> rt);

    virtual ~PassGeometry();

    // Render the pass. This should only be called by the pgTechnique.
    virtual void update(pgRenderEventArgs& e);
    virtual void render(pgRenderEventArgs& e);
    virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
    virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

    virtual bool meshFilter(pgMesh* mesh);
};

#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"

using namespace Diligent;

namespace ade
{

class PassTransparent : public pgPassRender
{
    typedef pgPassRender base;

  public:
    PassTransparent(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                    std::shared_ptr<pgPipeline> pipeline, std::vector<pgLight>* lights);

    virtual ~PassTransparent();

    virtual void Visit(pgMesh& mesh, pgPipeline* pipeline);
};

}    // namespace ade
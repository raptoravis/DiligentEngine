#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "test/pass/passrender.h"

using namespace Diligent;

namespace ade
{

class PassTransparent : public PassRender
{
    typedef PassRender base;

  public:
    PassTransparent(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                    std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights);

    virtual ~PassTransparent();

    virtual void Visit(Mesh& mesh, Pipeline* pipeline);
};

}    // namespace ade
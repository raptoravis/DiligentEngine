#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"

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
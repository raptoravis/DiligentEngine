#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PipelineDispatch;

class PassDispatch : public PassPilpeline
{
    typedef PassPilpeline base;

	bool m_bTransitionResources;
  public:
    PassDispatch(Technique* parentTechnique, std::shared_ptr<PipelineDispatch> pipeline);
    virtual ~PassDispatch();

    virtual void Dispatch();

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);

	void SetTransitionResources(bool bTransitionResources);
};
}    // namespace ade
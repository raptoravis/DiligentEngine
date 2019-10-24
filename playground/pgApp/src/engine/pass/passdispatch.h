#pragma once

#include "SampleBase.h"

#include "../engine.h"

class PipelineDispatch;

class PassDispatch : public pgPassPilpeline
{
    typedef pgPassPilpeline base;

  public:
    PassDispatch(pgTechnique* parentTechnique, std::shared_ptr<PipelineDispatch> pipeline);
    virtual ~PassDispatch();

	virtual void Dispatch();

    virtual void PreRender();
    virtual void Render(pgPipeline* pipeline);
};

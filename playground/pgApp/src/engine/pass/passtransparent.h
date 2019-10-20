#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class PassTransparent : public pgPassRender {
	typedef pgPassRender base;

public:
    PassTransparent(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
               const std::vector<pgLight>& lights);

	virtual ~PassTransparent();

	virtual void Visit(pgMesh& mesh);
};


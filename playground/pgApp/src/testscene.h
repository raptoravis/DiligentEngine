#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"
#include "engine/render/light.h"

class TestScene : public pgSceneAss {
public:
	TestScene(const pgSceneCreateInfo& sci)
		: pgSceneAss(sci)
	{
	}

	void customMesh();
};

#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"
#include "engine/utils/light.h"

class SceneTest : public pgSceneAss {
public:
	SceneTest()
		: pgSceneAss()
	{
	}

	void customMesh();
};

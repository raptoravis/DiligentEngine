#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"

class SceneTest : public pgSceneAss {
public:
	SceneTest()
		: pgSceneAss()
	{
	}

	void customMesh();
};

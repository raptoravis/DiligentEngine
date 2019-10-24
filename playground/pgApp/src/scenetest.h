#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"

class SceneTest : public ade::pgSceneAss
{
  public:
    SceneTest() : ade::pgSceneAss() {}

    void customMesh();
};

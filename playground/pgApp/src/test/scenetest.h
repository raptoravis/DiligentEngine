#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"

class SceneTest : public ade::SceneAss
{
  public:
    SceneTest() : ade::SceneAss() {}

    void customMesh();
};

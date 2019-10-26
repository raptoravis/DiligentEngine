#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../pass/passgltfmodel.h"


using namespace Diligent;
using namespace ade;

class TechniqueGdr : public Technique
{
    typedef Technique base;

    std::shared_ptr<ConstantBuffer> m_VSConstants;

    std::shared_ptr<Scene> m_pSceneCube;

  public:
    TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueGdr();

	void init();
    virtual void Update();
    virtual void Render();
};

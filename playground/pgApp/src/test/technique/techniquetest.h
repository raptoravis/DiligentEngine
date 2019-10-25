#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../pass/passgltfmodel.h"


using namespace Diligent;
using namespace ade;

class TechniqueTest : public Technique
{
    typedef Technique base;

    std::shared_ptr<ConstantBuffer> m_VSConstants;

    std::shared_ptr<Scene> m_pSceneCube;
    std::shared_ptr<Scene> m_pSceneCubeTex;

	std::shared_ptr<PassGltf> m_pGLTFPass;
    bool m_bGltfEnabled;
  public:
    TechniqueTest(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueTest();

	virtual void Update();
    virtual void Render();
};


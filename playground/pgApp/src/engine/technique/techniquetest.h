#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

namespace ade
{

class TechniqueTest : public Technique
{
    typedef Technique base;

    std::shared_ptr<ConstantBuffer> m_VSConstants;

    std::shared_ptr<Scene> m_pSceneCube;
    std::shared_ptr<Scene> m_pSceneCubeTex;

  public:
    TechniqueTest(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueTest();

    virtual void Render();
};

}    // namespace ade
#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../pass/passgltfmodel.h"


using namespace Diligent;

class TechniqueGdr : public ade::Technique
{
    typedef ade::Technique base;

    std::shared_ptr<ade::ConstantBuffer> m_PerObject;
    std::shared_ptr<ade::ConstantBuffer> m_colors;

    std::shared_ptr<ade::Scene> m_pSceneCube;

  public:
    TechniqueGdr(std::shared_ptr<ade::RenderTarget> rt, std::shared_ptr<ade::Texture> backBuffer);
    virtual ~TechniqueGdr();

	void init();
    virtual void Update();
    virtual void Render();
};

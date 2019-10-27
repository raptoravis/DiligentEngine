#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../pass/passgltfmodel.h"


using namespace Diligent;

class SceneGdr;

class TechniqueGdr : public ade::Technique
{
    typedef ade::Technique base;

    std::shared_ptr<ade::ConstantBuffer> m_PerObject;
    std::shared_ptr<ade::ConstantBuffer> m_materialId;
    std::shared_ptr<ade::ConstantBuffer> m_colors;

    std::shared_ptr<SceneGdr> m_pSceneGdr;

    std::shared_ptr<ade::Pass> createPassGdr(std::shared_ptr<ade::Scene> scene);
	
  public:
    TechniqueGdr(std::shared_ptr<ade::RenderTarget> rt, std::shared_ptr<ade::Texture> backBuffer);
    virtual ~TechniqueGdr();

	void init();
    virtual void Update();
    virtual void Render();
};

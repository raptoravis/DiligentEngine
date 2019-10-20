#pragma once 

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

class TechniqueTest : public pgTechnique {
	typedef pgTechnique base;

	std::shared_ptr<ConstantBuffer>             m_VSConstants;

	std::shared_ptr<pgScene>					m_pSceneCube;
    std::shared_ptr<pgScene>					m_pSceneCubeTex;

  public:
	TechniqueTest(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueTest();

	virtual void Render();
};
#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"

#include "../pipeline/pipelinelightback.h"
#include "../pipeline/pipelinelightdir.h"
#include "../pipeline/pipelinelightfront.h"

using namespace Diligent;

__declspec(align(16)) struct ScreenToViewParams {
    float4x4 m_InverseProjectionMatrix;
    float2 m_ScreenDimensions;
    float2 padding;
};

__declspec(align(16)) struct LightParams {
    uint32_t m_LightIndex;
    uint32_t padding[3];
};

class PassLight : public pgPass
{
    typedef pgPass base;

  public:
    static const char* kLightIndexBuffer;

  protected:
    std::shared_ptr<pgRenderTarget> m_pGBufferRT;
    std::vector<pgLight>* m_pLights;

    std::shared_ptr<PipelineLightFront> m_LightPipeline0;
    std::shared_ptr<PipelineLightBack> m_LightPipeline1;
    std::shared_ptr<PipelineLightDir> m_DirectionalLightPipeline;

    std::shared_ptr<pgScene> m_pPointLightScene;
    std::shared_ptr<pgScene> m_pSpotLightScene;
    std::shared_ptr<pgScene> m_pDirectionalLightScene;

    std::shared_ptr<pgPassPilpeline> m_pSubPassSphere0;
    std::shared_ptr<pgPassPilpeline> m_pSubPassSphere1;
    std::shared_ptr<pgPassPilpeline> m_pSubPassSpot0;
    std::shared_ptr<pgPassPilpeline> m_pSubPassSpot1;
    std::shared_ptr<pgPassPilpeline> m_pSubPassDir;

    std::shared_ptr<pgTechnique> m_pTechniqueSphere;
    std::shared_ptr<pgTechnique> m_pTechniqueSpot;
    std::shared_ptr<pgTechnique> m_pTechniqueDir;

    void createBuffers();

    void updateLightParams(const LightParams& lightParam, const pgLight& light);
    void updateScreenToViewParams();

  public:
    PassLight(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> pGBufferRT,
              std::shared_ptr<PipelineLightFront> front, std::shared_ptr<PipelineLightBack> back,
              std::shared_ptr<PipelineLightDir> dir, std::vector<pgLight>* Lights);

    virtual ~PassLight();

    virtual void PreRender();
    virtual void Render(pgPipeline* pipeline);
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(pgScene& scene, pgPipeline* pipeline);
    virtual void Visit(pgSceneNode& node, pgPipeline* pipeline);
};

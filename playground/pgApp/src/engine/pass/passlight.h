#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"

#include "../pipeline/pipelinelightback.h"
#include "../pipeline/pipelinelightdir.h"
#include "../pipeline/pipelinelightfront.h"

using namespace Diligent;

namespace ade
{

__declspec(align(16)) struct ScreenToViewParams {
    float4x4 m_InverseProjectionMatrix;
    float2 m_ScreenDimensions;
    float2 padding;
};

__declspec(align(16)) struct LightParams {
    uint32_t m_LightIndex;
    uint32_t padding[3];
};

class PassLight : public Pass
{
    typedef Pass base;

  public:
    static const char* kLightIndexBuffer;

  protected:
    std::shared_ptr<RenderTarget> m_pGBufferRT;
    std::vector<Light>* m_pLights;

    std::shared_ptr<PipelineLightFront> m_LightPipeline0;
    std::shared_ptr<PipelineLightBack> m_LightPipeline1;
    std::shared_ptr<PipelineLightDir> m_DirectionalLightPipeline;

    std::shared_ptr<Scene> m_pPointLightScene;
    std::shared_ptr<Scene> m_pSpotLightScene;
    std::shared_ptr<Scene> m_pDirectionalLightScene;

    std::shared_ptr<PassPilpeline> m_pSubPassSphere0;
    std::shared_ptr<PassPilpeline> m_pSubPassSphere1;
    std::shared_ptr<PassPilpeline> m_pSubPassSpot0;
    std::shared_ptr<PassPilpeline> m_pSubPassSpot1;
    std::shared_ptr<PassPilpeline> m_pSubPassDir;

    std::shared_ptr<Technique> m_pTechniqueSphere;
    std::shared_ptr<Technique> m_pTechniqueSpot;
    std::shared_ptr<Technique> m_pTechniqueDir;

    void createBuffers();

    void updateLightParams(const LightParams& lightParam, const Light& light);
    void updateScreenToViewParams();

  public:
    PassLight(Technique* parentTechnique, std::shared_ptr<RenderTarget> pGBufferRT,
              std::shared_ptr<PipelineLightFront> front, std::shared_ptr<PipelineLightBack> back,
              std::shared_ptr<PipelineLightDir> dir, std::vector<Light>* Lights);

    virtual ~PassLight();

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(Scene& scene, Pipeline* pipeline);
    virtual void Visit(SceneNode& node, Pipeline* pipeline);
};
}    // namespace ade
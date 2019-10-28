#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../pass/passgdr.h"

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
    void SetColorsConstantBufferData(PassGdr::Colors& data);
    //////////////////////////////////////////////////////////////////////////
    std::shared_ptr<ade::Texture> m_hiZDepthBuffer;
    std::shared_ptr<ade::Texture> m_hiZBuffer;
    uint32_t m_noofHiZMips;
    std::shared_ptr<ade::Buffer> m_drawcallInstanceCounts;
    std::shared_ptr<ade::Buffer> m_instancePredicates;
    std::shared_ptr<ade::Buffer> m_instanceBoundingBoxes;
    std::shared_ptr<ade::Buffer> m_instanceBuffer;
    std::shared_ptr<ade::Buffer> m_culledInstanceBuffer;
    std::shared_ptr<ade::Buffer> m_indirectBuffer;

    std::shared_ptr<ade::Pipeline> m_programMainPass;
    std::shared_ptr<ade::Pipeline> m_programOcclusionPass;
    std::shared_ptr<ade::Pipeline> m_programCopyZ;
    std::shared_ptr<ade::Pipeline> m_programDownscaleHiZ;
    std::shared_ptr<ade::Pipeline> m_programOccludeProps;
    std::shared_ptr<ade::Pipeline> m_programStreamCompaction;

    void createHiZBuffers();

    std::shared_ptr<ade::Pipeline> loadProgram(const std::string& shader,
                                               ade::Shader::ShaderType st,
                                               std::shared_ptr<ade::RenderTarget> rt,
                                               std::shared_ptr<ade::Scene> scene);

  public:
    TechniqueGdr(std::shared_ptr<ade::RenderTarget> rt, std::shared_ptr<ade::Texture> backBuffer);
    virtual ~TechniqueGdr();

    void init();
    virtual void Update();
    virtual void Render();
};

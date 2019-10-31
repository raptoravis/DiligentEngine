#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "../mesh/meshprop.h"
#include "../pass/passgdr.h"

using namespace Diligent;

class SceneGdr;

__declspec(align(16)) struct CBColors {
    Diligent::float4 colors[32];
};

__declspec(align(16)) struct CBMatrix {
    Diligent::float4x4 u_viewProj;
};


class TechniqueGdr : public ade::Technique
{
    typedef ade::Technique base;

    std::shared_ptr<ade::ConstantBuffer> m_PerObject;
    std::shared_ptr<ade::ConstantBuffer> m_materialId;
    std::shared_ptr<ade::ConstantBuffer> u_colors;

    std::shared_ptr<ade::ConstantBuffer> u_viewProj;
    std::shared_ptr<ade::ConstantBuffer> u_color;

    std::vector<std::shared_ptr<ade::ConstantBuffer>> u_inputRTSize;
    std::shared_ptr<ade::ConstantBuffer> u_cullingConfig;
    std::shared_ptr<ade::SamplerState> s_texOcclusionDepth;

    std::shared_ptr<SceneGdr> m_pSceneGdr;
    std::shared_ptr<ade::Pass> createPassGdr(std::shared_ptr<ade::Scene> scene);
    void SetColorsConstantBufferData(PassGdr::Colors& data);
    //////////////////////////////////////////////////////////////////////////
    std::shared_ptr<ade::Texture> m_hiZDepthBuffer;
    // std::shared_ptr<ade::Texture> m_hiZBuffer;

    std::vector<std::shared_ptr<ade::Texture>> m_hiZBuffers;

    uint32_t m_noofHiZMips;
    std::shared_ptr<ade::Buffer> m_drawcallInstanceCounts;
    std::shared_ptr<ade::Buffer> m_instancePredicates;
    std::shared_ptr<ade::StructuredBuffer> m_instanceBoundingBoxes;
    std::shared_ptr<ade::StructuredBuffer> m_instanceBuffer;
    std::shared_ptr<ade::StructuredBuffer> m_culledInstanceBuffer;
    std::shared_ptr<ade::StructuredBuffer> m_indirectBuffer;

    std::shared_ptr<ade::Shader> m_programOccludeProps;
    std::shared_ptr<ade::Shader> m_programStreamCompaction;

    void createHiZBuffers();

    std::shared_ptr<ade::Shader> loadProgram(const std::string& shader, ade::Shader::ShaderType st);
    //////////////////////////////////////////////////////////////////////////
    PosVertex* m_allPropVerticesDataCPU;
    uint32_t* m_allPropIndicesDataCPU;
    uint32_t* m_indirectBufferDataCPU;

    bool m_useIndirect;
    bool m_firstFrame;

    std::shared_ptr<ade::Buffer> m_allPropsVertexbufferHandle;
    std::shared_ptr<ade::Buffer> m_allPropsIndexbufferHandle;
    std::shared_ptr<ade::Buffer> m_indirectBufferData;

    uint32_t m_hiZwidth = 1024;
    uint32_t m_hiZheight = 512;

    Diligent::float4x4 m_occlusionProj;
    Diligent::float4x4 m_mainView;

	std::shared_ptr<ade::Pipeline> m_pipelineOccusionPass;
    std::shared_ptr<ade::Pipeline> m_pipelineMainPass;

    bool m_bDebug = false;

    std::vector<std::shared_ptr<ade::Texture>> m_hizTexMips;

    // submit drawcalls for all passes
    void renderOcclusionBufferPass();

    void renderDownscalePass();

    void renderOccludePropsPass();

    void renderMainPass();

    void initDebug();
    void initGdr();

    struct Data4Floats_t {
        float data[4];
    };

  public:
    TechniqueGdr(std::shared_ptr<ade::RenderTarget> rt, std::shared_ptr<ade::Texture> backBuffer);
    virtual ~TechniqueGdr();

    void init();
    virtual void Update();
    virtual void Render();
};

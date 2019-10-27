#pragma once

#include "engine/engine.h"

#define MODELS_COUNT 200u

class PassGdr : public ade::Pass
{
    typedef ade::Pass base;

  protected:
    std::shared_ptr<ade::Scene> m_pScene;
    std::shared_ptr<ade::Pipeline> m_pPipeline;

	Diligent::float4x4 m_nodeTransform;
  public:
    static const char* kPerObjectName;
    static const char* kColorsName;
    static const char* kMaterialIdName;
    

    PassGdr(ade::Technique* parentTechnique, std::shared_ptr<ade::Scene> scene,
            std::shared_ptr<ade::Pipeline> pipeline);
    virtual ~PassGdr();

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render(ade::Pipeline* pipeline);
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(ade::Scene& scene, ade::Pipeline* pipeline);
    virtual void Visit(ade::SceneNode& node, ade::Pipeline* pipeline);
    virtual void Visit(ade::Mesh& mesh, ade::Pipeline* pipeline);

    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        Diligent::float4x4 ViewProjection;
        Diligent::float4x4 Model;
        Diligent::float4x4 Models[MODELS_COUNT];
    };

    __declspec(align(16)) struct Colors {
        Diligent::float4 colors[32];
    };

    __declspec(align(16)) struct MaterialId {
        Diligent::Uint32 mid;
        Diligent::Uint32 padding[3];
    };

    void SetColorsConstantBufferData(Colors& data);
    void SetMaterialIdConstantBufferData(MaterialId& data);

    void SetPerObjectConstantBufferData(PerObject& perObjectData);

  private:
    std::shared_ptr<ade::ConstantBuffer> m_PerObjectConstantBuffer;
};

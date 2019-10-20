#pragma once

#include "../engine.h"

class TestPass : public pgPass
{
    typedef pgPass base;

  protected:
    std::shared_ptr<pgScene> m_pScene;
    std::shared_ptr<pgPipeline> m_pPipeline;

  public:
    const char* kPerObjectName = "Constants";

    TestPass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
             std::shared_ptr<pgPipeline> pipeline);
    virtual ~TestPass();

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render();
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(pgScene& scene);
    virtual void Visit(pgSceneNode& node);
    virtual void Visit(pgMesh& mesh);

  protected:
    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        Diligent::float4x4 ModelViewProjection;
    };

    // Set and bind the constant buffer data.
    void SetPerObjectConstantBufferData(PerObject& perObjectData);
    // Bind the constant to the shader.
    void BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader);

  private:
    std::shared_ptr<ConstantBuffer> m_PerObjectConstantBuffer;
};


#pragma once

#include "engine.h"

// Base pass provides implementations for functions used by most passes.
class BasePass : public pgPass
{
    typedef pgPass base;

  protected:
    std::shared_ptr<pgScene> m_pScene;
    std::shared_ptr<pgPipeline> m_pPipeline;

  public:
    const char* kPerObjectName = "PerObject";
    const char* kMaterialName = "Material";
    const char* kLightsName = "Lights";

    BasePass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
             std::shared_ptr<pgPipeline> pipeline);
    virtual ~BasePass();

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
        Diligent::float4x4 ModelView;
    };

    // Set and bind the constant buffer data.
    void SetPerObjectConstantBufferData(PerObject& perObjectData);
    // Bind the constant to the shader.
    void BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader);
};

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


// A pass that renders the opaque geometry in the scene.
class OpaquePass : public BasePass
{
  public:
    typedef BasePass base;

    OpaquePass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
               std::shared_ptr<pgPipeline> pipeline);
    virtual ~OpaquePass();

    virtual void Visit(pgMesh& mesh);

  protected:
  private:
};

#pragma once

#include "engine/engine.h"

namespace ade
{

class TestPass : public Pass
{
    typedef Pass base;

  protected:
    std::shared_ptr<Scene> m_pScene;
    std::shared_ptr<Pipeline> m_pPipeline;

  public:
    const char* kPerObjectName = "Constants";

    TestPass(Technique* parentTechnique, std::shared_ptr<Scene> scene,
             std::shared_ptr<Pipeline> pipeline);
    virtual ~TestPass();

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(Scene& scene, Pipeline* pipeline);
    virtual void Visit(SceneNode& node, Pipeline* pipeline);
    virtual void Visit(Mesh& mesh, Pipeline* pipeline);

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


}    // namespace ade
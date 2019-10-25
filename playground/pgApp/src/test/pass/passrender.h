#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

namespace ade
{

class PassRender : public PassPilpeline
{
    typedef PassPilpeline base;

  public:
    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        float4x4 ModelViewProjection;
        float4x4 ModelView;
    };

    static const char* kPerObjectName;
    static const char* kMaterialName;
    static const char* kLightsName;
    static const char* kScreenToViewParams;

  protected:
    std::vector<Light>* m_pLights;

    // Set and bind the constant buffer data.
    void SetPerObjectConstantBufferData(PerObject& perObjectData);
    // Bind the constant to the shader.
    void BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader);

    void BindMaterialConstantBuffer(std::shared_ptr<Shader> shader);

    void SetLightsBufferData(std::vector<Light>& lights);

    void BindLightsBuffer(std::shared_ptr<Shader> shader);

    void SetMaterialData(Material* mat);

  public:
    PassRender::PassRender(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                               std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights);
    virtual ~PassRender();

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(Scene& scene, Pipeline* pipeline);
    virtual void Visit(SceneNode& node, Pipeline* pipeline);
    virtual void Visit(Mesh& mesh, Pipeline* pipeline);
};

}    // namespace ade
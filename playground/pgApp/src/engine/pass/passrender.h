#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "../utils/light.h"

using namespace Diligent;

class pgPassRender : public pgPassPilpeline
{
    typedef pgPassPilpeline base;

  public:
    const char* kPerObjectName = "PerObject";
    const char* kMaterialName = "Material";
    const char* kLightsName = "Lights";

    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        float4x4 ModelViewProjection;
        float4x4 ModelView;
    };

  protected:
    std::vector<pgLight> m_Lights;

    // Set and bind the constant buffer data.
    void SetPerObjectConstantBufferData(PerObject& perObjectData);
    // Bind the constant to the shader.
    void BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader);

    void BindMaterialConstantBuffer(std::shared_ptr<Shader> shader);

    void SetLightsBufferData(std::vector<pgLight>& lights);

    void BindLightsBuffer(std::shared_ptr<Shader> shader);

	void SetMaterialData(pgMaterial* mat);
  public:
    pgPassRender::pgPassRender(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
                               const std::vector<pgLight>& lights);
    virtual ~pgPassRender();

    // Render the pass. This should only be called by the pgTechnique.
    virtual void update(pgRenderEventArgs& e);
    virtual void render(pgRenderEventArgs& e);
    virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
    virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render();
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(pgScene& scene);
    virtual void Visit(pgSceneNode& node);
    virtual void Visit(pgMesh& mesh);
};

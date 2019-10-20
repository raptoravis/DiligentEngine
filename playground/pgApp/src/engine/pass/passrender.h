#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "../utils/light.h"

using namespace Diligent;

class pgPassRender : public pgPassPilpeline
{
    typedef pgPassPilpeline base;

  public:
    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        float4x4 ModelViewProjection;
        float4x4 ModelView;
    };

	static const char* kPerObjectName;
    static const char* kMaterialName;
    static const char* kLightsName;

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

    // Render the pass. This should only be called by the RenderTechnique.
    virtual void PreRender();
    virtual void Render();
    virtual void PostRender();

    // Inherited from Visitor
    virtual void Visit(pgScene& scene);
    virtual void Visit(pgSceneNode& node);
    virtual void Visit(pgMesh& mesh);
};

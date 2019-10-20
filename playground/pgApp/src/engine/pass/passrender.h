#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

struct pgPassRenderCreateInfo {
    std::shared_ptr<pgScene>	scene;
    std::shared_ptr<pgPipeline>	pipeline;

	std::shared_ptr<ConstantBuffer>		PerObjectConstants;
    std::shared_ptr<ConstantBuffer>		MaterialConstants;
    std::shared_ptr<StructuredBuffer>	LightsStructuredBuffer;
};

class pgPassRender : public pgPass {
	typedef pgPass base;

  public:
    // PerObject constant buffer data.
    __declspec(align(16)) struct PerObject {
        float4x4 ModelViewProjection;
        float4x4 ModelView;
    };

  protected:
    std::shared_ptr<ConstantBuffer>		m_PerObjectConstants;
    std::shared_ptr<ConstantBuffer>		m_MaterialConstants;
    std::shared_ptr<StructuredBuffer>	m_LightsStructuredBuffer;

	// Set and bind the constant buffer data.
    void SetPerObjectConstantBufferData(PerObject& perObjectData);
    // Bind the constant to the shader.
    void BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader);

	void SetMaterialConstantBufferData(pgMaterial::MaterialProperties& materialData);

    void BindMaterialConstantBuffer(std::shared_ptr<Shader> shader);

  public:
    pgPassRender(const pgPassRenderCreateInfo& ci);
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


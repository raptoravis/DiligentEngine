#pragma once

#include "SampleBase.h"
#include "BasicMath.h"

#include "MapHelper.h"

#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "CommonlyUsedStates.h"
#include "ShaderMacroHelper.h"
#include "FileSystem.h"
#include "imgui.h"
#include "imGuIZMO.h"
#include "InputController.h"

class Object
{
public:
	virtual ~Object() {
		//
	}
};

class Camera : public Object
{
	Diligent::MouseState m_LastMouseState;

	Diligent::float4x4   m_cameraTransform;
	Diligent::float3 pos;
	Diligent::float3 look;

	const Diligent::float3 up = { 0.0f, 1.0f, 0.0f };

public:
	Camera();

	virtual ~Camera();

	void reset();

	void update(Diligent::InputController* pInputController, float ElapsedTime);

	const Diligent::float3& getPos() const {
		return pos;
	}

	const Diligent::float3& getLook() const {
		return look;
	}

	const Diligent::float4x4& getTransform() const {
		return m_cameraTransform;
	}

};

class Scene : public Object {
	//
};

class RenderEventArgs
{
public:
	void* Caller;
	float CurrTime;
	float ElapsedTime;

	Camera* pCamera;
public:
	RenderEventArgs()	{
	}

	void set(void* caller, float currentTime, float elapsedTime, Camera* camera) {
		Caller = caller;
		pCamera = camera;
		CurrTime = currentTime;
		ElapsedTime = elapsedTime;
	}

};


class pgPass : public Object
{
	bool m_bEnabled;
public:
	pgPass() : m_bEnabled(true) {
		//
	}

	// Enable or disable the pass. If a pass is disabled, the technique will skip it.
	void SetEnabled(bool enabled) {
		m_bEnabled = enabled;
	}

	bool IsEnabled() const {
		return m_bEnabled;
	}

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e) = 0;
	virtual void Render(RenderEventArgs& e) = 0;
};

class pgTechnique : public Object
{
public:
	pgTechnique();
	virtual ~pgTechnique();

	// Add a pass to the technique. The ID of the added pass is returned
	// and can be used to retrieve the pass later.
	virtual unsigned int AddPass(pgPass* pass);
	virtual pgPass* GetPass(unsigned int ID) const;

	// Render the scene using the passes that have been configured.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);

private:
	typedef std::vector<pgPass*> RenderPassList;
	RenderPassList m_Passes;

};

// Base pass provides implementations for functions used by most passes.
class pgBasePass : public pgPass
{
protected:
	Diligent::RefCntAutoPtr<Diligent::IRenderDevice>		  m_pDevice;
	Diligent::RefCntAutoPtr<Diligent::IDeviceContext>		  m_pImmediateContext;
	Diligent::RefCntAutoPtr<Diligent::IEngineFactory>         m_pEngineFactory;

public:
	typedef pgPass base;

	pgBasePass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory);
	virtual ~pgBasePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);
	virtual void UpdateUI(RenderEventArgs& e);
};
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

class RenderEventArgs
{
	const Object& Caller;

public:
	RenderEventArgs(const Object& caller, float fDeltaTime, 
		float fTotalTime, uint64_t frameCounter, 
		Camera* camera = nullptr, Diligent::IPipelineState* pipelineState = nullptr) : Caller(caller)
		, ElapsedTime(fDeltaTime)
		, TotalTime(fTotalTime)
		, FrameCounter(frameCounter)
		, Camera(camera)
		, PipelineState(pipelineState)
	{}

	float ElapsedTime;
	float TotalTime;
	int64_t FrameCounter;
	Camera* Camera;
	Diligent::IPipelineState* PipelineState;
};


class pgPass : public Object
{
	// Enable or disable the pass. If a pass is disabled, the technique will skip it.
	virtual void SetEnabled(bool enabled) = 0;
	virtual bool IsEnabled() const = 0;

	// Render the pass. This should only be called by the RenderTechnique.
	virtual void PreRender(RenderEventArgs& e) = 0;
	virtual void Render(RenderEventArgs& e) = 0;
	virtual void PostRender(RenderEventArgs& e) = 0;
};

class pgTechnique : public Object
{
public:
	pgTechnique();
	virtual ~pgTechnique();

	// Add a pass to the technique. The ID of the added pass is returned
	// and can be used to retrieve the pass later.
	virtual unsigned int AddPass(std::shared_ptr<pgPass> pass);
	virtual std::shared_ptr<pgPass> GetPass(unsigned int ID) const;

	// Render the scene using the passes that have been configured.
	virtual void Render(RenderEventArgs& renderEventArgs);

protected:

private:
	typedef std::vector< std::shared_ptr<pgPass> > RenderPassList;
	RenderPassList m_Passes;

};
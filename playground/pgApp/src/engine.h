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

class Object
{
	//
};

class Camera : public Object
{
	//
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
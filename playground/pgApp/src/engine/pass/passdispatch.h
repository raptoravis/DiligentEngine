#pragma once

#include "SampleBase.h"

#include "../engine.h"


class PassDispatch : public pgPass
{
	typedef pgPass base;
public:
	PassDispatch(std::shared_ptr<Shader> computeShader, const Diligent::uint3& numGroups);
	virtual ~PassDispatch();

	void SetNumGroups(const Diligent::uint3& numGroups);
	Diligent::uint3 GetNumGroups() const;

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);

private:

	std::shared_ptr<Shader> m_pComputeShader;

	// The number of groups to dispatch for the compute shader kernel.
	Diligent::uint3 m_NumGroups;

};

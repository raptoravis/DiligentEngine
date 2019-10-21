#pragma once

#include "SampleBase.h"

#include "../engine.h"


class PassDispatch : public pgPass
{
    typedef pgPass base;

  public:
    PassDispatch(pgTechnique* parentTechnique, std::shared_ptr<Shader> computeShader,
                 const Diligent::uint3& numGroups);
    virtual ~PassDispatch();

    void SetNumGroups(const Diligent::uint3& numGroups);
    Diligent::uint3 GetNumGroups() const;

    virtual void Render();

  private:
    std::shared_ptr<Shader> m_pComputeShader;

    // The number of groups to dispatch for the compute shader kernel.
    Diligent::uint3 m_NumGroups;
};

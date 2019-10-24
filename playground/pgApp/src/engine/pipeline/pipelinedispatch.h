#pragma once

#include "engine/engine.h"

namespace ade
{

class PipelineDispatch : public Pipeline
{
    typedef Pipeline base;

  private:
    std::shared_ptr<Shader> m_pComputeShader;

    // The number of groups to dispatch for the compute shader kernel.
    Diligent::uint3 m_NumGroups;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineDispatch(const Diligent::uint3& numGroups);
    virtual ~PipelineDispatch();

    // void SetNumGroups(const Diligent::uint3& numGroups);
    const Diligent::uint3 GetNumGroups() const;

    virtual void Bind();
    virtual void UnBind();
};

}    // namespace ade
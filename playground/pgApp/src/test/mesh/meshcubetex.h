#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

namespace ade
{

class MeshCubeTex : public Mesh
{
    void CreateVertexBuffer();
    void CreateIndexBuffer();

  public:
    MeshCubeTex();
    virtual ~MeshCubeTex();
};

}    // namespace ade
#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

class MeshCubeTex : public ade::Mesh
{
    void CreateVertexBuffer();
    void CreateIndexBuffer();

  public:
    MeshCubeTex();
    virtual ~MeshCubeTex();
};

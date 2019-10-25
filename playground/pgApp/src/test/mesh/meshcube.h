#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

class MeshCube : public ade::Mesh
{
    void CreateVertexBuffer();
    void CreateIndexBuffer();

  public:
    MeshCube();
    virtual ~MeshCube();
};

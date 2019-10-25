#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

class MeshProp : public ade::Mesh
{
    void CreateVertexBuffer();
    void CreateIndexBuffer();

  public:
    MeshProp();
    virtual ~MeshProp();
};

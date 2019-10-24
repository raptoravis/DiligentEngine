#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class MeshCube : public Mesh
{
    void CreateVertexBuffer();
    void CreateIndexBuffer();

  public:
    MeshCube();
    virtual ~MeshCube();
};
}    // namespace ade
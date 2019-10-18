#pragma once

#include "SampleBase.h"

#include "../engine.h"

class MeshCube : public pgMesh {
	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	MeshCube();
	virtual ~MeshCube();

	virtual void render(pgRenderEventArgs& e);
};
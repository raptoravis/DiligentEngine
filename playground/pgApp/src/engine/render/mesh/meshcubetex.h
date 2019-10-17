#pragma once

#include "SampleBase.h"

#include "../../engine.h"

class MeshCubeTex : public pgMesh {
	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	MeshCubeTex();
	virtual ~MeshCubeTex();

	virtual void render(pgRenderEventArgs& e);
};
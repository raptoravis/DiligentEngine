#pragma once

#include "SampleBase.h"

#include "../engine.h"

class MeshCubeTex : public pgMesh {
	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	MeshCubeTex(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx);
	virtual ~MeshCubeTex();

	virtual void render(pgRenderEventArgs& e);
};
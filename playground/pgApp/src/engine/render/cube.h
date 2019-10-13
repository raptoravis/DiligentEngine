#pragma once

#include "SampleBase.h"

#include "../engine.h"

class Cube : public pgMesh {
	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	Cube(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx);
	virtual ~Cube();

	virtual void Render(pgSceneNode* sceneNode, pgRenderEventArgs& renderEventArgs);
};
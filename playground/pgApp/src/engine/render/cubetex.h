#pragma once

#include "SampleBase.h"

#include "../engine.h"

class CubeTex : public pgMesh {
	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	CubeTex(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx);
	virtual ~CubeTex();

	virtual void Render(pgSceneNode* sceneNode, pgRenderEventArgs& renderEventArgs);
};
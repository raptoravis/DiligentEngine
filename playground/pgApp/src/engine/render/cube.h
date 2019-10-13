#pragma once

#include "SampleBase.h"

#include "../engine.h"

class Cube : public pgMesh {
	Diligent::RefCntAutoPtr<Diligent::IBuffer>                m_CubeVertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>                m_CubeIndexBuffer;

	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	Cube(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx);
	virtual ~Cube();

	virtual void Render(pgRenderEventArgs& renderEventArgs);
};
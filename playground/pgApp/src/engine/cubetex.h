#pragma once

#include "SampleBase.h"

#include "engine.h"

class CubeTex : public Mesh {
	Diligent::RefCntAutoPtr<Diligent::IBuffer>                m_CubeVertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>                m_CubeIndexBuffer;

	void CreateVertexBuffer();
	void CreateIndexBuffer();

public:
	CubeTex(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx);
	virtual ~CubeTex();

	virtual void Render(RenderEventArgs& renderEventArgs);
};

#include "cubetex.h"

using namespace Diligent;

CubeTex::CubeTex(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx) 
	: pgMesh(device, ctx)
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

CubeTex::~CubeTex() {
	//
}

void CubeTex::CreateVertexBuffer()
{
	// Layout of this structure matches the one we defined in the pipeline state
	struct Vertex
	{
		float3 pos;
		float2 uv;
	};

	// Cube vertices

	//      (-1,+1,+1)________________(+1,+1,+1) 
	//               /|              /|
	//              / |             / |
	//             /  |            /  |
	//            /   |           /   |
	//(-1,-1,+1) /____|__________/(+1,-1,+1)
	//           |    |__________|____| 
	//           |   /(-1,+1,-1) |    /(+1,+1,-1)
	//           |  /            |   /
	//           | /             |  /
	//           |/              | /
	//           /_______________|/ 
	//        (-1,-1,-1)       (+1,-1,-1)
	// 

	// This time we have to duplicate verices because texture coordinates cannot
	// be shared
	Vertex CubeVerts[] =
	{
		{float3(-1,-1,-1), float2(0,1)},
		{float3(-1,+1,-1), float2(0,0)},
		{float3(+1,+1,-1), float2(1,0)},
		{float3(+1,-1,-1), float2(1,1)},

		{float3(-1,-1,-1), float2(0,1)},
		{float3(-1,-1,+1), float2(0,0)},
		{float3(+1,-1,+1), float2(1,0)},
		{float3(+1,-1,-1), float2(1,1)},

		{float3(+1,-1,-1), float2(0,1)},
		{float3(+1,-1,+1), float2(1,1)},
		{float3(+1,+1,+1), float2(1,0)},
		{float3(+1,+1,-1), float2(0,0)},

		{float3(+1,+1,-1), float2(0,1)},
		{float3(+1,+1,+1), float2(0,0)},
		{float3(-1,+1,+1), float2(1,0)},
		{float3(-1,+1,-1), float2(1,1)},

		{float3(-1,+1,-1), float2(1,0)},
		{float3(-1,+1,+1), float2(0,0)},
		{float3(-1,-1,+1), float2(0,1)},
		{float3(-1,-1,-1), float2(1,1)},

		{float3(-1,-1,+1), float2(1,1)},
		{float3(+1,-1,+1), float2(0,1)},
		{float3(+1,+1,+1), float2(0,0)},
		{float3(-1,+1,+1), float2(1,0)}
	};

	BufferDesc VertBuffDesc;
	VertBuffDesc.Name = "Cube vertex buffer";
	VertBuffDesc.Usage = USAGE_STATIC;
	VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = sizeof(CubeVerts);
	BufferData VBData;
	VBData.pData = CubeVerts;
	VBData.DataSize = sizeof(CubeVerts);
	m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_CubeVertexBuffer);
}

void CubeTex::CreateIndexBuffer()
{
	Uint32 Indices[] =
	{
		2,0,1,    2,3,0,
		4,6,5,    4,7,6,
		8,10,9,   8,11,10,
		12,14,13, 12,15,14,
		16,18,17, 16,19,18,
		20,21,22, 20,22,23
	};

	BufferDesc IndBuffDesc;
	IndBuffDesc.Name = "Cube index buffer";
	IndBuffDesc.Usage = USAGE_STATIC;
	IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = sizeof(Indices);
	BufferData IBData;
	IBData.pData = Indices;
	IBData.DataSize = sizeof(Indices);
	m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_CubeIndexBuffer);
}

void CubeTex::Render(pgRenderEventArgs& renderEventArgs) {
	// Bind vertex and index buffers
	Uint32 offset = 0;
	IBuffer *pBuffs[] = { m_CubeVertexBuffer };
	m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
	m_pImmediateContext->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	DrawAttribs DrawAttrs;
	DrawAttrs.IsIndexed = true;      // This is an indexed draw call
	DrawAttrs.IndexType = VT_UINT32; // Index type
	DrawAttrs.NumIndices = 36;
	// Verify the state of vertex and index buffers
	DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
	m_pImmediateContext->Draw(DrawAttrs);
}

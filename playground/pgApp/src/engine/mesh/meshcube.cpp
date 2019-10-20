#include "meshcube.h"
#include "../scene/sceneass.h"

using namespace Diligent;

MeshCube::MeshCube() 
	: pgMesh()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

MeshCube::~MeshCube() {
	//
}

void MeshCube::CreateVertexBuffer()
{
	// Layout of this structure matches the one we defined in the pipeline state
	struct Vertex
	{
		float3 pos;
		float4 color;
	};

	// MeshCube vertices

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

	const Vertex MeshCubeVerts[8] =
	{
		{float3(-1,-1,-1), float4(1,0,0,1)},
		{float3(-1,+1,-1), float4(0,1,0,1)},
		{float3(+1,+1,-1), float4(0,0,1,1)},
		{float3(+1,-1,-1), float4(1,1,1,1)},

		{float3(-1,-1,+1), float4(1,1,0,1)},
		{float3(-1,+1,+1), float4(0,1,1,1)},
		{float3(+1,+1,+1), float4(1,0,1,1)},
		{float3(+1,-1,+1), float4(0.2f,0.2f,0.2f,1)},
	};

	std::shared_ptr<pgBuffer> buffer = pgSceneAss::createFloatVertexBuffer(pgApp::s_device, 
		(const float*)MeshCubeVerts, 8, sizeof(Vertex));

	pgBufferBinding binding{"VERTEX", 0};
	addVertexBuffer(binding, buffer);
}

void MeshCube::CreateIndexBuffer()
{
	const Uint32 Indices[] =
	{
		2,0,1, 2,3,0,
		4,6,5, 4,7,6,
		0,7,4, 0,3,7,
		1,0,4, 1,4,5,
		1,5,2, 5,6,2,
		3,6,7, 3,2,6
	};

	m_pIndexBuffer = pgSceneAss::createUIntIndexBuffer(pgApp::s_device,
		Indices, 36);
}


#include "meshcube.h"
#include "engine/scene/sceneass.h"

using namespace Diligent;

MeshCube::MeshCube() : ade::Mesh()
{
    CreateVertexBuffer();
    CreateIndexBuffer();
}

MeshCube::~MeshCube()
{
    //
}

void MeshCube::CreateVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex {
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

    const Vertex MeshCubeVerts[8] = {
        { float3(-1, -1, -1), float4(1, 0, 0, 1) },
        { float3(-1, +1, -1), float4(0, 1, 0, 1) },
        { float3(+1, +1, -1), float4(0, 0, 1, 1) },
        { float3(+1, -1, -1), float4(1, 1, 1, 1) },

        { float3(-1, -1, +1), float4(1, 1, 0, 1) },
        { float3(-1, +1, +1), float4(0, 1, 1, 1) },
        { float3(+1, +1, +1), float4(1, 0, 1, 1) },
        { float3(+1, -1, +1), float4(0.2f, 0.2f, 0.2f, 1) },
    };

    std::shared_ptr<ade::Buffer> buffer = ade::Scene::CreateVertexBufferFloat(
        ade::App::s_device, (float*)MeshCubeVerts, 8, sizeof(Vertex));

    ade::BufferBinding binding{ "VERTEX", 0 };
    AddVertexBuffer(binding, buffer);
}

void MeshCube::CreateIndexBuffer()
{
    const Uint32 Indices[] = { 2, 0, 1, 2, 3, 0, 4, 6, 5, 4, 7, 6, 0, 7, 4, 0, 3, 7,
                               1, 0, 4, 1, 4, 5, 1, 5, 2, 5, 6, 2, 3, 6, 7, 3, 2, 6 };

    m_pIndexBuffer = ade::Scene::CreateIndexBufferUInt(ade::App::s_device, (uint32_t*)Indices, 36);
}

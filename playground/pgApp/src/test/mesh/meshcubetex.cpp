
#include "meshcubetex.h"
#include "engine/scene/sceneass.h"

using namespace Diligent;

MeshCubeTex::MeshCubeTex() : ade::Mesh()
{
    CreateVertexBuffer();
    CreateIndexBuffer();
}

MeshCubeTex::~MeshCubeTex()
{
    //
}

void MeshCubeTex::CreateVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex {
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
    const Vertex CubeVerts[] = {
        { float3(-1, -1, -1), float2(0, 1) }, { float3(-1, +1, -1), float2(0, 0) },
        { float3(+1, +1, -1), float2(1, 0) }, { float3(+1, -1, -1), float2(1, 1) },

        { float3(-1, -1, -1), float2(0, 1) }, { float3(-1, -1, +1), float2(0, 0) },
        { float3(+1, -1, +1), float2(1, 0) }, { float3(+1, -1, -1), float2(1, 1) },

        { float3(+1, -1, -1), float2(0, 1) }, { float3(+1, -1, +1), float2(1, 1) },
        { float3(+1, +1, +1), float2(1, 0) }, { float3(+1, +1, -1), float2(0, 0) },

        { float3(+1, +1, -1), float2(0, 1) }, { float3(+1, +1, +1), float2(0, 0) },
        { float3(-1, +1, +1), float2(1, 0) }, { float3(-1, +1, -1), float2(1, 1) },

        { float3(-1, +1, -1), float2(1, 0) }, { float3(-1, +1, +1), float2(0, 0) },
        { float3(-1, -1, +1), float2(0, 1) }, { float3(-1, -1, -1), float2(1, 1) },

        { float3(-1, -1, +1), float2(1, 1) }, { float3(+1, -1, +1), float2(0, 1) },
        { float3(+1, +1, +1), float2(0, 0) }, { float3(-1, +1, +1), float2(1, 0) }
    };

    std::shared_ptr<ade::Buffer> buffer = ade::SceneAss::createFloatVertexBuffer(
        ade::App::s_device, (const float*)CubeVerts, 24, sizeof(Vertex));

    ade::BufferBinding binding{ "VERTEX", 0 };
    addVertexBuffer(binding, buffer);
}

void MeshCubeTex::CreateIndexBuffer()
{
    Uint32 Indices[] = { 2,  0,  1,  2,  3,  0,  4,  6,  5,  4,  7,  6,  8,  10, 9,  8,  11, 10,
                         12, 14, 13, 12, 15, 14, 16, 18, 17, 16, 19, 18, 20, 21, 22, 20, 22, 23 };

    m_pIndexBuffer = ade::SceneAss::createUIntIndexBuffer(ade::App::s_device, Indices, 36);
}

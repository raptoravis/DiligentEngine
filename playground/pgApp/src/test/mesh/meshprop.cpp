#include "meshprop.h"
#include "engine/scene/sceneass.h"

using namespace Diligent;


struct PosVertex {
    float m_x;
    float m_y;
    float m_z;

    static LayoutElement LayoutElems[];
};

LayoutElement PosVertex::LayoutElems[] = {
    // Attribute 0 - vertex position
    LayoutElement{ 0, 0, 3, VT_FLOAT32, False }
};


static PosVertex s_cubeVertices[8] = {
    { -0.5f, 0.5f, 0.5f },  { 0.5f, 0.5f, 0.5f },  { -0.5f, -0.5f, 0.5f },  { 0.5f, -0.5f, 0.5f },
    { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f },
};

static const uint16_t s_cubeIndices[36] = {
    0, 1, 2,             // 0
    1, 3, 2, 4, 6, 5,    // 2
    5, 6, 7, 0, 2, 4,    // 4
    4, 2, 6, 1, 5, 3,    // 6
    5, 7, 3, 0, 4, 1,    // 8
    4, 5, 1, 2, 3, 6,    // 10
    6, 3, 7,
};

struct RenderPass {
    enum Enum { Occlusion = 1 << 0, MainPass = 1 << 1, All = Occlusion | MainPass };
};

// All the per-instance data we store
struct InstanceData {
    float m_world[16];
    float m_bboxMin[4];
    float m_bboxMax[4];
};

// A description of each prop
struct Prop {
    PosVertex* m_vertices;
    uint16_t* m_indices;
    InstanceData* m_instances;

    std::shared_ptr<ade::Buffer> m_vertexbufferHandle;
    std::shared_ptr<ade::Buffer> m_indexbufferHandle;

    uint16_t m_noofVertices;
    uint16_t m_noofIndices;
    uint16_t m_noofInstances;
    uint16_t m_materialID;
    RenderPass::Enum m_renderPass;
};

// A simplistic material, comprised of a color only
struct Material {
    float m_color[4];
};

inline void setVector4(float* dest, float x, float y, float z, float w)
{
    dest[0] = x;
    dest[1] = y;
    dest[2] = z;
    dest[3] = w;
}

// Sets up a prop
void createCubeMesh(Prop& prop)
{
    prop.m_noofVertices = 8;
    prop.m_noofIndices = 36;
    prop.m_vertices = new PosVertex[prop.m_noofVertices];
    prop.m_indices = new uint16_t[prop.m_noofIndices];

    memcpy(prop.m_vertices, s_cubeVertices, prop.m_noofVertices * sizeof(PosVertex));
    memcpy(prop.m_indices, s_cubeIndices, prop.m_noofIndices * sizeof(uint16_t));

    //   std::shared_ptr<ade::Buffer> buffer = ade::SceneAss::createFloatVertexBuffer(
    //       ade::App::s_device, (const float*)CubeVerts, 24, sizeof(Vertex));

    //   ade::BufferBinding binding{ "VERTEX", 0 };
    //   addVertexBuffer(binding, buffer);

    //   m_pIndexBuffer = ade::SceneAss::createUIntIndexBuffer(ade::App::s_device, Indices, 36);

    // prop.m_vertexbufferHandle = bgfx::createVertexBuffer(
    //       bgfx::makeRef(prop.m_vertices, prop.m_noofVertices * PosVertex::ms_layout.getStride()),
    //       PosVertex::ms_layout);

    //   prop.m_indexbufferHandle = bgfx::createIndexBuffer(
    //       bgfx::makeRef(prop.m_indices, prop.m_noofIndices * sizeof(uint16_t)));
}

// returns a random number between 0 and 1
float rand01()
{
    return rand() / (float)RAND_MAX;
}


MeshProp::MeshProp() : ade::Mesh() {}

MeshProp::~MeshProp()
{
    //
}

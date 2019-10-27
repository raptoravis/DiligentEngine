#include "meshprop.h"
#include "engine/scene/sceneass.h"

using namespace Diligent;


LayoutElement PosVertex::LayoutElems[] = {
    // Attribute 0 - vertex position
    LayoutElement{ 0, 0, 3, VT_FLOAT32, False }
};


static PosVertex s_cubeVertices[8] = {
    { -0.5f, 0.5f, 0.5f },  { 0.5f, 0.5f, 0.5f },  { -0.5f, -0.5f, 0.5f },  { 0.5f, -0.5f, 0.5f },
    { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f },
};

static const uint32_t s_cubeIndices[36] = {
    0, 1, 2,             // 0
    1, 3, 2, 4, 6, 5,    // 2
    5, 6, 7, 0, 2, 4,    // 4
    4, 2, 6, 1, 5, 3,    // 6
    5, 7, 3, 0, 4, 1,    // 8
    4, 5, 1, 2, 3, 6,    // 10
    6, 3, 7,
};

void setVector4(float4& dest, float x, float y, float z, float w)
{
    dest.x = x;
    dest.y = y;
    dest.z = z;
    dest.w = w;
}

// Sets up a prop
void createCubeMesh(Prop& prop)
{
    prop.m_noofVertices = 8;
    prop.m_noofIndices = 36;
    prop.m_vertices = new PosVertex[prop.m_noofVertices];
    prop.m_indices = new uint32_t[prop.m_noofIndices];

    memcpy(prop.m_vertices, s_cubeVertices, prop.m_noofVertices * sizeof(PosVertex));
    memcpy(prop.m_indices, s_cubeIndices, prop.m_noofIndices * sizeof(uint32_t));

    std::shared_ptr<ade::Buffer> buffer = ade::SceneAss::createFloatVertexBuffer(
        ade::App::s_device, (const float*)prop.m_vertices, prop.m_noofVertices, sizeof(PosVertex));

    prop.m_vertexbufferHandle = buffer;

    prop.m_indexbufferHandle = ade::SceneAss::createUIntIndexBuffer(
        ade::App::s_device, prop.m_indices, prop.m_noofIndices);
}

// returns a random number between 0 and 1
float rand01()
{
    return rand() / (float)RAND_MAX;
}


MeshProp::MeshProp(const Prop* prop) : ade::Mesh()
{
    if (!prop) {
        createCubeMesh(m_prop);
    } else {
        m_prop = *prop;
    }
	
	{

        ade::BufferBinding binding{ "VERTEX", 0 };
        AddVertexBuffer(binding, m_prop.m_vertexbufferHandle);

        m_pIndexBuffer = m_prop.m_indexbufferHandle;
    }
}

MeshProp::~MeshProp()
{
    //
}

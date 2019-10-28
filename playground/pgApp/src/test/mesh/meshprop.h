#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

struct PosVertex {
    float m_x;
    float m_y;
    float m_z;

    static Diligent::LayoutElement LayoutElems[];
};

struct RenderPass {
    enum Enum { Occlusion = 1 << 0, MainPass = 1 << 1, All = Occlusion | MainPass };
};

// All the per-instance data we store
struct InstanceData {
    // float m_world[16];
    // float m_bboxMin[4];
    // float m_bboxMax[4];
    Diligent::float4x4 m_world;
    Diligent::float4 m_bboxMin;
    Diligent::float4 m_bboxMax;
};

// A description of each prop
struct Prop {
    PosVertex* m_vertices;
    uint32_t* m_indices;
    InstanceData* m_instances;

    std::shared_ptr<ade::Buffer> m_vertexbufferHandle;
    std::shared_ptr<ade::Buffer> m_indexbufferHandle;

    uint16_t m_noofVertices;
    uint16_t m_noofIndices;
    uint16_t m_noofInstances;
    uint16_t m_materialID;
    ::RenderPass::Enum m_renderPass;
};

// A simplistic material, comprised of a color only
struct MaterialColor {
    // float m_color[4];
    Diligent::float4 m_color;
};

void setVector4(Diligent::float4& dest, float x, float y, float z, float w);
void createCubeMesh(Prop& prop);

float rand01();

class MeshProp : public ade::Mesh
{
  public:
    MeshProp(const Prop* prop);
    virtual ~MeshProp();

    Prop m_prop;
};

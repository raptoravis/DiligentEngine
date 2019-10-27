#include "passgdr.h"
#include "../mesh/meshprop.h"

using namespace ade;

const char* PassGdr::kPerObjectName = "PerObject";
const char* PassGdr::kColorsName = "Colors";
const char* PassGdr::kMaterialIdName = "MaterialId";

PassGdr::PassGdr(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                 std::shared_ptr<Pipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
}

PassGdr::~PassGdr() {}

void PassGdr::SetColorsConstantBufferData(Colors& data)
{
    auto cb =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(PassGdr::kColorsName));

    cb->Set(data);
}

void PassGdr::SetMaterialIdConstantBufferData(MaterialId& data)
{
    auto cb =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(PassGdr::kMaterialIdName));

    cb->Set(data);
}

void PassGdr::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(PassGdr::kPerObjectName));

    perObjectCB->Set(perObjectData);
}


void PassGdr::PreRender()
{
    if (m_pPipeline) {
        m_pPipeline->Bind();
    }
}

void PassGdr::Render(Pipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}

void PassGdr::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void PassGdr::Visit(Scene& scene, Pipeline* pipeline) {}

void PassGdr::Visit(SceneNode& node, Pipeline* pipeline)
{
    m_nodeTransform = node.GetWorldTransfom();
}

void PassGdr::Visit(Mesh& mesh, Pipeline* pipeline)
{
    Camera* camera = App::s_eventArgs.pCamera;

    // Diligent::float4x4 is column major,
    // in hlsl, when mul(mat, vec), the mat is column major like opengl
    Diligent::float4x4 viewMatrix = camera->getViewMatrix();
    Diligent::float4x4 projMatrix = camera->getProjectionMatrix();

    // the mat1 * mat2 should be left mul
    Diligent::float4x4 viewProjMatrix = viewMatrix * projMatrix;

	MeshProp* pMeshProp = (MeshProp*)&mesh;
	//////////////////////////////////////////////////////////////////////////
    auto prop = pMeshProp->m_prop;
    InstanceData* data = prop.m_instances;

    const uint32_t numInstances = prop.m_noofInstances;

    PerObject perObjectData;

	for (uint32_t i = 0; i < numInstances; ++i) {
        perObjectData.Models[i] = data[i].m_world;
    }

    perObjectData.Model = m_nodeTransform;
    perObjectData.ViewProjection = viewProjMatrix;

    // Update the constant buffer data
    SetPerObjectConstantBufferData(perObjectData);
    mesh.SetInstancesCount((uint32_t)numInstances);
    //////////////////////////////////////////////////////////////////////////
    PassGdr::MaterialId materialId;

    materialId.mid = prop.m_materialID;

    SetMaterialIdConstantBufferData(materialId);

    mesh.Render(pipeline);
}

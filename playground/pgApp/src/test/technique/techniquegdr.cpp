#include "techniquegdr.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"

#include "../mesh/meshprop.h"

#include "../pass/passgdr.h"
#include "../scene/scenegdr.h"


using namespace ade;

TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    {
        m_pSceneGdr = std::make_shared<SceneGdr>();
        m_pSceneGdr->create();

        auto prop0 = m_pSceneGdr->m_props[3];
        std::shared_ptr<MeshProp> meshProp = std::make_shared<MeshProp>(&prop0);

        auto scene = std::make_shared<Scene>();
        float4x4 trans1 = float4x4::Identity();
        std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
        root1->AddMesh(meshProp);
        scene->SetRootNode(root1);

        //////////////////////////////////////////////////////////////////////////
        m_PerObject = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::PerObject));
        m_materialId = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::MaterialId));
        m_colors = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::Colors));

        this->Set(PassGdr::kPerObjectName, m_PerObject);
        this->Set(PassGdr::kMaterialIdName, m_materialId);
        this->Set(PassGdr::kColorsName, m_colors);
		//////////////////////////////////////////////////////////////////////////
        std::shared_ptr<Pass> pPass = createPassGdr(scene);

        AddPass(pPass);

        //
        {
            auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
            auto dstTexture = m_pBackBuffer;

            std::shared_ptr<PassCopyTexture> pCopyTexPass =
                std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
            AddPass(pCopyTexPass);
        }
    }
}

std::shared_ptr<ade::Pass> TechniqueGdr::createPassGdr(std::shared_ptr<ade::Scene> scene)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(m_pRenderTarget);

    {
        std::shared_ptr<Shader> vs = std::make_shared<ade::Shader>();
        vs->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                               "vs_gdr_instanced_indirect_rendering.vsh", "main", "./gdr", false);

        std::shared_ptr<Shader> ps = std::make_shared<ade::Shader>();
        ps->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                               "fs_gdr_instanced_indirect_rendering.psh", "main", "./gdr", false);

        vs->GetShaderParameterByName(PassGdr::kPerObjectName).Set(m_PerObject);
        vs->GetShaderParameterByName(PassGdr::kMaterialIdName).Set(m_materialId);

        ps->GetShaderParameterByName(PassGdr::kColorsName).Set(m_colors);

        pipeline->SetShader(ade::Shader::Shader::VertexShader, vs);
        pipeline->SetShader(ade::Shader::Shader::PixelShader, ps);

        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
            // LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
            //               sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
        };

        pipeline->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    std::shared_ptr<PassGdr> pPass = std::make_shared<PassGdr>(this, scene, pipeline);

    PassGdr::Colors colors;

    uint32_t noofMaterials = m_pSceneGdr->m_noofMaterials;
    noofMaterials = std::min(noofMaterials, 32u);
    memcpy(colors.colors, m_pSceneGdr->m_materials,
           sizeof(m_pSceneGdr->m_materials[0]) * noofMaterials);
    pPass->SetColorsConstantBufferData(colors);

    return pPass;
}


TechniqueGdr::~TechniqueGdr()
{
    //
}

void TechniqueGdr::Render()
{
    // const float rotSpeed = (Diligent::PI_F / 180.0f) * 100.0f;
    //{
    //    auto rootCube = m_pScene->GetRootNode();
    //    auto local = rootCube->GetLocalTransform();
    //    auto localNew =
    //        Diligent::float4x4::RotationY(rotSpeed * App::s_eventArgs.ElapsedTime) * local;
    //    rootCube->SetLocalTransform(localNew);
    //}

    base::Render();
}

void TechniqueGdr::Update()
{
    ImGui::Separator();

    bool bTemp = false;
    ImGui::Checkbox("debug", &bTemp);
    ImGui::Separator();
}

void TechniqueGdr::init() {}
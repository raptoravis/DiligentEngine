#include "techniquegdr.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"

#include "../pipeline/pipelinegdr.h"

#include "../mesh/meshprop.h"

#include "../pass/passgdr.h"

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
        std::shared_ptr<MeshProp> mesh = std::make_shared<MeshProp>();

#if RIGHT_HANDED
        float z = 8.0f;
#else
        float z = 8.0f;
#endif
        float4x4 trans1 = float4x4::RotationX(-PI_F * 0.1f) * float4x4::Translation(0.f, 0.0f, z);
        std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
        root1->addMesh(mesh);
        m_pSceneCube = std::make_shared<Scene>();
        m_pSceneCube->setRootNode(root1);

#if RIGHT_HANDED
        z = 5.0f;
#else
        z = 5.0f;
#endif

        //
        m_PerObject = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::PerObject));
        m_colors = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::ColorsMaterial));

        this->Set(PipelineGdr::kPerObjectName, m_PerObject);
        this->Set(PipelineGdr::kColorsMaterialName, m_colors);

        std::shared_ptr<PipelineGdr> pipeline =
            std::make_shared<PipelineGdr>(m_pRenderTarget, m_PerObject, m_colors);

        std::shared_ptr<PassGdr> pPass =
            std::make_shared<PassGdr>(this, m_pSceneCube, pipeline);
        AddPass(pPass);

		PassGdr::ColorsMaterial colorsMaterial;
        colorsMaterial.mid = 0;
        colorsMaterial.colors[0] = {1,1,1,1};

		pPass->SetColorsMaterialPerObjectConstantBufferData(colorsMaterial);

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


TechniqueGdr::~TechniqueGdr()
{
    //
}

void TechniqueGdr::Render()
{
    //const float rotSpeed = (Diligent::PI_F / 180.0f) * 100.0f;
    //{
    //    auto rootCube = m_pSceneCube->getRootNode();
    //    auto local = rootCube->getLocalTransform();
    //    auto localNew =
    //        Diligent::float4x4::RotationY(rotSpeed * App::s_eventArgs.ElapsedTime) * local;
    //    rootCube->setLocalTransform(localNew);
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
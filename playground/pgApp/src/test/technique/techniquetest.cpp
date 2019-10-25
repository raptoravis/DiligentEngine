#include "techniquetest.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"

#include "../pipeline/pipelinecolorvertex.h"
#include "../pipeline/pipelinetexvertex.h"

#include "../mesh/meshcube.h"
#include "../mesh/meshcubetex.h"
#include "../pass/passgltfmodel.h"

#include "../pass/passtest.h"


TechniqueTest::TechniqueTest(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer), m_bGltfEnabled(false)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    bool bLoad = m_bGltfEnabled;
    m_pGLTFPass = std::make_shared<PassGltf>(this, m_pRenderTarget, bLoad);
    m_pGLTFPass->SetEnabled(bLoad);
    AddPass(m_pGLTFPass);

    {
        std::shared_ptr<MeshCube> meshCube = std::make_shared<MeshCube>();
        std::shared_ptr<MeshCubeTex> meshCubeTex = std::make_shared<MeshCubeTex>();

#if RIGHT_HANDED
        float z = 8.0f;
#else
        float z = 8.0f;
#endif
        float4x4 trans1 = float4x4::RotationX(-PI_F * 0.1f) * float4x4::Translation(0.f, 0.0f, z);
        std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
        root1->addMesh(meshCube);
        m_pSceneCube = std::make_shared<Scene>();
        m_pSceneCube->setRootNode(root1);

#if RIGHT_HANDED
        z = 5.0f;
#else
        z = 5.0f;
#endif

        float4x4 trans2 = float4x4::Scale(0.6f) * float4x4::RotationX(-PI_F * 0.1f) *
                          float4x4::Translation(0.f, 0.0f, z);
        std::shared_ptr<SceneNode> root2 = std::make_shared<SceneNode>(trans2);
        root2->addMesh(meshCubeTex);
        m_pSceneCubeTex = std::make_shared<Scene>();
        m_pSceneCubeTex->setRootNode(root2);

        //
        m_VSConstants = std::make_shared<ConstantBuffer>((uint32_t)sizeof(float4x4));
        this->Set("Constants", m_VSConstants);

        std::shared_ptr<PipelineColorVertex> pipelineColorVertex =
            std::make_shared<PipelineColorVertex>(m_pRenderTarget);
        std::shared_ptr<PipelineTexVertex> pipelineTexVertex =
            std::make_shared<PipelineTexVertex>(m_pRenderTarget);

        std::shared_ptr<TestPass> pCubePass =
            std::make_shared<TestPass>(this, m_pSceneCube, pipelineColorVertex);
        AddPass(pCubePass);

        std::shared_ptr<TestPass> pCubeTexPass =
            std::make_shared<TestPass>(this, m_pSceneCubeTex, pipelineTexVertex);
        AddPass(pCubeTexPass);

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


TechniqueTest::~TechniqueTest()
{
    //
}

void TechniqueTest::Render()
{
    const float rotSpeed = (Diligent::PI_F / 180.0f) * 100.0f;
    {
        auto rootCube = m_pSceneCube->getRootNode();
        auto local = rootCube->getLocalTransform();
        auto localNew =
            Diligent::float4x4::RotationY(rotSpeed * App::s_eventArgs.ElapsedTime) * local;
        rootCube->setLocalTransform(localNew);
    }
    {
        auto rootCube = m_pSceneCubeTex->getRootNode();
        auto local = rootCube->getLocalTransform();
        auto localNew =
            Diligent::float4x4::RotationY(-rotSpeed * App::s_eventArgs.ElapsedTime) * local;
        rootCube->setLocalTransform(localNew);
    }

    base::Render();
}

void TechniqueTest::Update()
{
    ImGui::Separator();

    bool bGltfEnabled = m_pGLTFPass->IsEnabled();
    ImGui::Checkbox("gltf", &m_bGltfEnabled);
    ImGui::Separator();

    if (!bGltfEnabled && m_bGltfEnabled) {
        m_pGLTFPass->Load();
    } else if (bGltfEnabled && !bGltfEnabled) {
        //
    }

	m_pGLTFPass->SetEnabled(m_bGltfEnabled);

    m_pGLTFPass->UpdateUI();
}


#include "techniquetest.h"

#include "engine/mesh/meshcube.h"
#include "engine/mesh/meshcubetex.h"
#include "engine/pass/passgltfmodel.h"

#include "engine/pipeline/pipelinecolorvertex.h"
#include "engine/pipeline/pipelinetexvertex.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"


TechniqueTest::TechniqueTest(std::shared_ptr<pgRenderTarget> rt,
                             std::shared_ptr<pgTexture> backBuffer)
    : base(rt, backBuffer)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(m_pRT);
    addPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(m_pRT);
    addPass(pClearRTPass);

    bool bTestGltf = false;
    if (bTestGltf) {
        std::shared_ptr<PassGltf> pGLTFPass = std::make_shared<PassGltf>();
        addPass(pGLTFPass);
    }

    {
        std::shared_ptr<MeshCube> meshCube = std::make_shared<MeshCube>();
        std::shared_ptr<MeshCubeTex> meshCubeTex = std::make_shared<MeshCubeTex>();

        float4x4 trans1 =
            float4x4::RotationX(-PI_F * 0.1f) * float4x4::Translation(0.f, 0.0f, 8.0f);
        std::shared_ptr<pgSceneNode> root1 = std::make_shared<pgSceneNode>(trans1);
        root1->addMesh(meshCube);
        m_pSceneCube = std::make_shared<pgScene>();
        m_pSceneCube->setRootNode(root1);

        float4x4 trans2 = float4x4::Scale(0.6f) * float4x4::RotationX(-PI_F * 0.1f) *
                          float4x4::Translation(0.f, 0.0f, 5.0f);
        std::shared_ptr<pgSceneNode> root2 = std::make_shared<pgSceneNode>(trans2);
        root2->addMesh(meshCubeTex);
        m_pSceneCubeTex = std::make_shared<pgScene>();
        m_pSceneCubeTex->setRootNode(root2);

        //
        pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_PEROBJECT] = "Constants";
        m_VSConstants = std::make_shared<ConstantBuffer>((uint32_t)sizeof(float4x4));
        pgApp::s_reources[pgApp::RESOURCE_SLOT_PEROBJECT] = m_VSConstants;

        std::shared_ptr<PipelineColorVertex> pipelineColorVertex =
            std::make_shared<PipelineColorVertex>(m_pRT);
        std::shared_ptr<PipelineTexVertex> pipelineTexVertex =
            std::make_shared<PipelineTexVertex>(m_pRT);

        std::shared_ptr<TestPass> pCubePass =
            std::make_shared<TestPass>(m_pSceneCube, pipelineColorVertex);
        addPass(pCubePass);

        std::shared_ptr<TestPass> pCubeTexPass =
            std::make_shared<TestPass>(m_pSceneCubeTex, pipelineTexVertex);
        addPass(pCubeTexPass);

        //
        {
            auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
            auto dstTexture = m_pBackBuffer;

            std::shared_ptr<PassCopyTexture> pCopyTexPass =
                std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
            addPass(pCopyTexPass);
        }
    }
}


TechniqueTest::~TechniqueTest()
{
    //
}

void TechniqueTest::update(pgRenderEventArgs& e)
{
    base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueTest::render(pgRenderEventArgs& e)
{
    base::render(e);
}

void TechniqueTest::bind(pgRenderEventArgs& e, pgBindFlag flag)
{
    //
}

void TechniqueTest::unbind(pgRenderEventArgs& e, pgBindFlag flag)
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
            Diligent::float4x4::RotationY(rotSpeed * pgApp::s_eventArgs.ElapsedTime) * local;
        rootCube->setLocalTransform(localNew);
    }
    {
        auto rootCube = m_pSceneCubeTex->getRootNode();
        auto local = rootCube->getLocalTransform();
        auto localNew =
            Diligent::float4x4::RotationY(-rotSpeed * pgApp::s_eventArgs.ElapsedTime) * local;
        rootCube->setLocalTransform(localNew);
    }

    base::Render();
}

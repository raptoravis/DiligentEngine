#include "techniquetest.h"

#include "engine/render/pass/passgltfmodel.h"
#include "engine/render/mesh/meshcube.h"
#include "engine/render/mesh/meshcubetex.h"

#include "engine/render/pipeline/pipelinecolorvertex.h"
#include "engine/render/pipeline/pipelinetexvertex.h"

#include "engine/render/pass/passsetrt.h"
#include "engine/render/pass/passclearrt.h"
#include "engine/render/pass/passcopytexture.h"

TechniqueTest::TechniqueTest(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
	pgPassCreateInfo pci{ *(pgCreateInfo*)&ci };
	pgSceneCreateInfo sci{ *(pgCreateInfo*)&ci };

	PassSetRTCreateInfo psrtci{ *(pgCreateInfo*)&ci };
	psrtci.rt = m_pRT;
	std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(psrtci);
	addPass(pSetRTPass);

	PassClearRTCreateInfo pcrtci{ *(pgCreateInfo*)&ci };
	pcrtci.rt = m_pRT;
	std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(pcrtci);
	addPass(pClearRTPass);

	bool bTestGltf = false;
	if (bTestGltf) {
		std::shared_ptr<PassGltf> pGLTFPass = std::make_shared<PassGltf>(pci);
		addPass(pGLTFPass);
	}

	{
		std::shared_ptr<MeshCube> meshCube = std::make_shared<MeshCube>(m_pDevice, m_pImmediateContext);
		std::shared_ptr<MeshCubeTex> meshCubeTex = std::make_shared<MeshCubeTex>(m_pDevice, m_pImmediateContext);

		float4x4 trans1 = float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 8.0f);
		std::shared_ptr<pgSceneNode> root1 = std::make_shared<pgSceneNode>(trans1);
		root1->addMesh(meshCube);
		std::shared_ptr<pgScene> sceneCube = std::make_shared<pgScene>(sci);
		sceneCube->setRootNode(root1);

		float4x4 trans2 = float4x4::Scale(0.6f) * float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 5.0f);
		std::shared_ptr<pgSceneNode> root2 = std::make_shared<pgSceneNode>(trans2);
		root2->addMesh(meshCubeTex);
		std::shared_ptr<pgScene> sceneCubeTex = std::make_shared<pgScene>(sci);
		sceneCubeTex->setRootNode(root2);

		pgPipelineCreateInfo plci{ *(pgCreateInfo*)&ci };
		std::shared_ptr<PipelineColorVertex> pipelineColorVertex = std::make_shared<PipelineColorVertex>(plci);
		std::shared_ptr<PipelineTexVertex> pipelineTexVertex = std::make_shared<PipelineTexVertex>(plci);

		pgPassPipelineCreateInfo ppci{ pci };

		ppci.scene = sceneCube;
		ppci.pipeline = pipelineColorVertex;
		std::shared_ptr<pgPassPilpeline> pCubePass = std::make_shared<pgPassPilpeline>(ppci);
		addPass(pCubePass);

		ppci.scene = sceneCubeTex;
		ppci.pipeline = pipelineTexVertex;
		std::shared_ptr<pgPassPilpeline> pCubeTexPass = std::make_shared<pgPassPilpeline>(ppci);
		addPass(pCubeTexPass);

		//
		{
			CopyTexturePassCreateInfo ctpci{ pci };
			ctpci.srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
			ctpci.dstTexture = m_pBackBuffer;

			std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(ctpci);
			addPass(pCopyTexPass);
		}
	}
}

TechniqueTest::~TechniqueTest() {

}

void TechniqueTest::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueTest::render(pgRenderEventArgs& e) {
	base::render(e);
}

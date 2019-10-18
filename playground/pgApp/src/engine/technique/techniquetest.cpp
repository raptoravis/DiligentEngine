#include "techniquetest.h"

#include "engine/pass/passgltfmodel.h"
#include "engine/mesh/meshcube.h"
#include "engine/mesh/meshcubetex.h"

#include "engine/pipeline/pipelinecolorvertex.h"
#include "engine/pipeline/pipelinetexvertex.h"

#include "engine/pass/passsetrt.h"
#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"


TechniqueTest::TechniqueTest(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer)
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

		float4x4 trans1 = float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 8.0f);
		std::shared_ptr<pgSceneNode> root1 = std::make_shared<pgSceneNode>(trans1);
		root1->addMesh(meshCube);
		std::shared_ptr<pgScene> sceneCube = std::make_shared<pgScene>();
		sceneCube->setRootNode(root1);

		float4x4 trans2 = float4x4::Scale(0.6f) * float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 5.0f);
		std::shared_ptr<pgSceneNode> root2 = std::make_shared<pgSceneNode>(trans2);
		root2->addMesh(meshCubeTex);
		std::shared_ptr<pgScene> sceneCubeTex = std::make_shared<pgScene>();
		sceneCubeTex->setRootNode(root2);

		std::shared_ptr<PipelineColorVertex> pipelineColorVertex = std::make_shared<PipelineColorVertex>(m_pRT);
		std::shared_ptr<PipelineTexVertex> pipelineTexVertex = std::make_shared<PipelineTexVertex>(m_pRT);

		std::shared_ptr<OpaquePass> pCubePass = std::make_shared<OpaquePass>(sceneCube, pipelineColorVertex);
		addPass(pCubePass);

		std::shared_ptr<pgPassPilpeline> pCubeTexPass = std::make_shared<pgPassPilpeline>(sceneCubeTex, pipelineTexVertex);
		addPass(pCubeTexPass);

		//
		{
			auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
			auto dstTexture = m_pBackBuffer;

			std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
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

void TechniqueTest::bind(pgRenderEventArgs& e, pgBindFlag flag) {

}

void TechniqueTest::unbind(pgRenderEventArgs& e, pgBindFlag flag) {

}

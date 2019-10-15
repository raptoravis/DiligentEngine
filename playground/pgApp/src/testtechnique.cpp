#include "TestTechnique.h"

#include "engine/render/gltfmodelpass.h"
#include "engine/render/meshcube.h"
#include "engine/render/meshcubetex.h"

#include "pipelinecolorvertex.h"
#include "pipelinetexvertex.h"

TestTechnique::TestTechnique(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
	pgPassCreateInfo pci{ *(pgCreateInfo*)&ci };
	pgSceneCreateInfo sci{ *(pgCreateInfo*)&ci };

	bool bTestGltf = false;
	if (bTestGltf) {
		std::shared_ptr<pgGLTFPass> pGLTFPass = std::make_shared<pgGLTFPass>(pci);
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

		pci.scene = sceneCube;
		pci.pipeline = pipelineColorVertex;
		std::shared_ptr<pgBasePass> pCubePass = std::make_shared<pgBasePass>(pci);
		addPass(pCubePass);

		pci.scene = sceneCubeTex;
		pci.pipeline = pipelineTexVertex;
		std::shared_ptr<pgBasePass> pCubeTexPass = std::make_shared<pgBasePass>(pci);
		addPass(pCubeTexPass);
	}
}

TestTechnique::~TestTechnique() {

}

void TestTechnique::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TestTechnique::render(pgRenderEventArgs& e) {
	base::render(e);
}

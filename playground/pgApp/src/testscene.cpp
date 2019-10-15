#include "testscene.h"


void TestScene::customMesh() {
	if (m_Meshes.size() > 0) {
		auto mesh = m_Meshes[0];
		auto mat = mesh->getMaterial();
		mat->SetOpacity(0.5f);
	}
}

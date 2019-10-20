#include "passtransparent.h"

PassTransparent::PassTransparent(const pgPassRenderCreateInfo& ci)
	: base(ci)
{
}

PassTransparent::~PassTransparent()
{
}

bool PassTransparent::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return bTransparent;
}

// Render a frame
void PassTransparent::render(pgRenderEventArgs& e) {
	m_pScene->_render(e);
}

void PassTransparent::update(pgRenderEventArgs& e) {
	//
}

void PassTransparent::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::bind(e, flag);
}

void PassTransparent::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::unbind(e, flag);
}

void PassTransparent::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && pMaterial->IsTransparent()) {
        mesh.Render();
    }
}

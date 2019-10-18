#include "engine.h"

pgTechnique::pgTechnique(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer)
		: m_pRT(rt)
		, m_pBackBuffer(backBuffer)
{
}

pgTechnique::~pgTechnique() {
	m_Passes.clear();
}

unsigned int pgTechnique::addPass(std::shared_ptr<pgPass> pass) {
	// No check for duplicate passes (it may be intended to render the same pass multiple times?)
	m_Passes.push_back(pass);
	return static_cast<unsigned int>(m_Passes.size()) - 1;
}

std::shared_ptr<pgPass> pgTechnique::getPass(unsigned int ID) const {
	if (ID < m_Passes.size())
	{
		return m_Passes[ID];
	}

	return 0;
}

unsigned int pgTechnique::addPass(std::shared_ptr<RenderPass> pass) {
	_m_Passes.push_back(pass);
	return static_cast<unsigned int>(_m_Passes.size()) - 1;
}

void pgTechnique::update(pgRenderEventArgs& e) {
	for (auto pass : m_Passes)
	{
		if (pass->isEnabled())
		{
			pass->update(e);
		}
	}
}

// Render the scene using the passes that have been configured.
void pgTechnique::_render(pgRenderEventArgs& e) {
	// keep to restore it
	auto oldTechnique = e.pTechnique;

	auto currentTechnique = this;
	e.pTechnique = currentTechnique;

	currentTechnique->bind(e, pgBindFlag::pgBindFlag_Technique);

	render(e);

	currentTechnique->unbind(e, pgBindFlag::pgBindFlag_Technique);
	e.pTechnique = oldTechnique;
}

void pgTechnique::render(pgRenderEventArgs& e) {
	for (auto pass : m_Passes)
	{
		if (pass->isEnabled())
		{
			pass->_render(e);
		}
	}
}

void pgTechnique::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgTechnique::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgTechnique::Render() {
	for (auto pass : _m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->PreRender();
			pass->Render();
			pass->PostRender();
		}
	}
}
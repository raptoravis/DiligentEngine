#include "passdispatch.h"


PassDispatch::PassDispatch(std::shared_ptr<Shader> computeShader, const Diligent::uint3& numGroups)
	: base(0)
	, m_pComputeShader(computeShader)
	, m_NumGroups(numGroups)
{}

PassDispatch::~PassDispatch()
{}

// Render a frame
void PassDispatch::render(pgRenderEventArgs& e) {
	m_pComputeShader->Dispatch(m_NumGroups);
}

void PassDispatch::update(pgRenderEventArgs& e) {
	//
}

void PassDispatch::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}
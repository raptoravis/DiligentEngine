#include "PassCopyBuffer.h"

PassCopyBuffer::PassCopyBuffer(std::shared_ptr<pgBuffer> dstBuffer, std::shared_ptr<pgBuffer> srcBuffer)
	: base(0)
	, m_dstBuffer(dstBuffer)
	, m_srcBuffer(srcBuffer)
{
}

PassCopyBuffer::~PassCopyBuffer()
{
}

// Render a frame
void PassCopyBuffer::render(pgRenderEventArgs& e) {
	
	pgApp::s_ctx->CopyBuffer(m_srcBuffer->GetBuffer(), 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, 
		m_dstBuffer->GetBuffer(), 0, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void PassCopyBuffer::update(pgRenderEventArgs& e) {
	//
}

void PassCopyBuffer::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}
#include "engine.h"
#include "./app.h"

#include <windows.h>

void ReportErrorAndThrow(const std::string& file, int line, const std::string& function, const std::string& message)
{
	std::stringstream ss;

	DWORD errorCode = GetLastError();
	LPTSTR errorMessage = nullptr;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, 0, nullptr);

	if (errorMessage)
	{
		ss << file << "(" << line << "): " << "error " << errorCode << ": " << errorMessage << std::endl;
		LocalFree(errorMessage);
	}
	else
	{
		ss << file << "(" << line << "): " << message << std::endl;
	}

	OutputDebugStringA(ss.str().c_str());
	MessageBoxA(nullptr, message.c_str(), function.c_str(), MB_ICONERROR);
	throw new std::exception(message.c_str());
}


pgPipeline::pgPipeline(const pgPipelineCreateInfo& ci) 
	: m_pDevice(ci.device)
	, m_pSwapChain(ci.swapChain)
	, m_pImmediateContext(ci.ctx)
	, m_pEngineFactory(ci.factory)
	, m_desc(ci.desc)
	, m_pRT(ci.rt)
{
	//
}

pgPipeline::~pgPipeline() {
	//
}

void pgPipeline::setRenderTarget() {
	if (m_pRT) {
		m_pRT->Bind();
	}
}

void pgPipeline::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	if (flag & pgBindFlag::pgBindFlag_Pipeline) {
		this->setRenderTarget();
	}

	e.pApp->bind(e, flag);

	if (flag & pgBindFlag::pgBindFlag_Pipeline) {
		// Set the pipeline state
		m_pImmediateContext->SetPipelineState(m_pPSO);

		// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
		// makes sure that resources are transitioned to required states.
		m_pImmediateContext->CommitShaderResources(m_pSRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
}

pgPassPilpeline::pgPassPilpeline(const pgPassPipelineCreateInfo& ci)
	: base(ci)
	, m_pPipeline(ci.pipeline)
{
	assert(m_pPipeline);
}

pgPassPilpeline::~pgPassPilpeline()
{
}

// Render a frame
void pgPassPilpeline::render(pgRenderEventArgs& e)
{
	m_pPipeline->bind(e, pgBindFlag_Pipeline);

	m_scene->render(e);
}

void pgPassPilpeline::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	m_pPipeline->bind(e, flag);
}

void pgPassPilpeline::update(pgRenderEventArgs& e)
{
}


pgTechnique::pgTechnique(const pgTechniqueCreateInfo& ci)
		: m_pDevice(ci.device)
		, m_pSwapChain(ci.swapChain)
		, m_pImmediateContext(ci.ctx)
		, m_pEngineFactory(ci.factory)
		, m_desc(ci.desc)
		, m_pRT(ci.rt)
		, m_pBackBuffer(ci.backBuffer)
{}

pgTechnique::~pgTechnique()
{
	m_Passes.clear();
}

unsigned int pgTechnique::addPass(std::shared_ptr<pgPass> pass)
{
	// No check for duplicate passes (it may be intended to render the same pass multiple times?)
	m_Passes.push_back(pass);
	return static_cast<unsigned int>(m_Passes.size()) - 1;
}

std::shared_ptr<pgPass> pgTechnique::getPass(unsigned int ID) const
{
	if (ID < m_Passes.size())
	{
		return m_Passes[ID];
	}

	return 0;
}

void pgTechnique::update(pgRenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->isEnabled())
		{
			pass->update(e);
		}
	}
}

// Render the scene using the passes that have been configured.
void pgTechnique::render(pgRenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->isEnabled())
		{
			// set the pass
			e.pPass = pass.get();
			e.pPass->bind(e, pgBindFlag::pgBindFlag_Pass);

			pass->render(e);

			// clear it
			e.pPass = 0;
		}
	}
}



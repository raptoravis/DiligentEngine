#include "engine.h"

#include <windows.h>

Diligent::RefCntAutoPtr<Diligent::IRenderDevice>	pgApp::s_device;
Diligent::RefCntAutoPtr<Diligent::IDeviceContext>	pgApp::s_ctx;
Diligent::RefCntAutoPtr<Diligent::ISwapChain>		pgApp::s_swapChain;
Diligent::RefCntAutoPtr<Diligent::IEngineFactory>	pgApp::s_engineFactory;
std::shared_ptr<pgRenderTarget>						pgApp::s_rt;
std::shared_ptr<pgTexture>							pgApp::s_backBuffer;

Diligent::SwapChainDesc								pgApp::s_desc;


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

pgBuffer::BufferType pgBuffer::GetType() const {
	return pgBuffer::Unknown;
}

// Bind the buffer for rendering.
bool pgBuffer::Bind(unsigned int id, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	return true;
}

// Unbind the buffer for rendering.
void pgBuffer::UnBind(unsigned int id, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	//
}

void pgBuffer::Copy(std::shared_ptr<pgBuffer> other) {
	//
}

pgPass::pgPass(std::shared_ptr<pgScene> scene)
	: m_bEnabled(true)
	, m_scene(scene)
{
}

pgPass::~pgPass() {
	//
}

void pgPass::_render(pgRenderEventArgs& e) {
	// keep to restore it
	auto oldPass = e.pPass;

	auto currentPass = this;
	e.pPass = currentPass;
	currentPass->bind(e, pgBindFlag::pgBindFlag_Pass);

	currentPass->render(e);

	currentPass->unbind(e, pgBindFlag::pgBindFlag_Pass);
	e.pPass = oldPass;
}

void pgPass::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgPass::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

pgPassPilpeline::pgPassPilpeline(std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline)
	: base(scene)
	, m_pPipeline(pipeline)
{
	assert(m_pPipeline);
}

pgPassPilpeline::~pgPassPilpeline()
{
}

// Render a frame
void pgPassPilpeline::render(pgRenderEventArgs& e) {
	m_scene->_render(e);
}

void pgPassPilpeline::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	m_pPipeline->bind(e, flag);
}

void pgPassPilpeline::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	m_pPipeline->unbind(e, flag);
}


void pgPassPilpeline::update(pgRenderEventArgs& e) {
}


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


pgApp::pgApp() {
	//
}


pgApp::~pgApp() {
	//
}

void pgApp::Initialize(Diligent::IEngineFactory*   pEngineFactory,
	Diligent::IRenderDevice*    pDevice,
	Diligent::IDeviceContext**  ppContexts,
	Diligent::Uint32            NumDeferredCtx,
	Diligent::ISwapChain*       pSwapChain)
{
	//
}

void pgApp::Render() {
	//
}

void pgApp::Update(double CurrTime, double ElapsedTime) {
	//
}

void pgApp::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgApp::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

#include "engine.h"

#include <windows.h>

Diligent::RefCntAutoPtr<Diligent::IRenderDevice>	pgApp::s_device;
Diligent::RefCntAutoPtr<Diligent::IDeviceContext>	pgApp::s_ctx;
Diligent::RefCntAutoPtr<Diligent::ISwapChain>		pgApp::s_swapChain;
Diligent::RefCntAutoPtr<Diligent::IEngineFactory>	pgApp::s_engineFactory;
std::shared_ptr<pgRenderTarget>						pgApp::s_rt;
std::shared_ptr<pgTexture>							pgApp::s_backBuffer;

Diligent::SwapChainDesc								pgApp::s_desc;
pgRenderEventArgs									pgApp::s_eventArgs;

std::shared_ptr<pgObject>							pgApp::s_reources[pgApp::RESOURCE_SLOT::RESOURCE_SLOT_COUNT];
const char*											pgApp::s_reourceNames[pgApp::RESOURCE_SLOT::RESOURCE_SLOT_COUNT] = {
	"PerObject", 
	"Material", 
	"Lights"
};

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

uint32_t pgObject::s_uuid = 0;

pgObject::pgObject()
	: m_uuid(++pgObject::s_uuid)
{
}

pgBuffer::pgBuffer(uint32_t stride, uint32_t count, Diligent::IBuffer* buffer)
	: m_uiCount(count)
	, m_uiStride(stride)
	, m_BindFlags(0)
	, m_bIsBound(false)
{
	if (buffer) {
		m_pBuffer.Attach(buffer);
		m_pBuffer->AddRef();
	}
}

Diligent::IBuffer* pgBuffer::GetBuffer() {
	return m_pBuffer;
}


uint32_t pgBuffer::getCount() const {
	return m_uiCount;
}


pgBuffer::BufferType pgBuffer::GetType() const {
	return pgBuffer::Unknown;
}

Diligent::IBufferView* pgBuffer::getUnorderedAccessView() {
	auto buffer = m_pBuffer;

	auto uav = buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
	return uav;
}

Diligent::IBufferView* pgBuffer::getShaderResourceView() {
	auto buffer = m_pBuffer;

	auto srv = buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
	return srv;
}

// Bind the buffer for rendering.
bool pgBuffer::Bind(unsigned int slot, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	auto buffer = m_pBuffer;

	Diligent::Uint32 offset[] = { 0 };
	Diligent::IBuffer *pBuffs[] = { buffer };
	const uint32_t buffs = 1;

	pgApp::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

	return true;
}

// Unbind the buffer for rendering.
void pgBuffer::UnBind(unsigned int id, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	//
}

void pgBuffer::Copy(std::shared_ptr<pgBuffer> other) {
	//
}

pgPass::pgPass(std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline)
	: m_bEnabled(true)
	, m_pScene(scene)
	, m_pPipeline(pipeline)
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

void pgPass::update(pgRenderEventArgs& e) {
	//
}

void pgPass::render(pgRenderEventArgs& e) {
	//
}


void pgPass::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgPass::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgPass::PreRender() {
	//
}

void pgPass::Render() {
	//
}

void pgPass::PostRender() {
	//
}

void pgPass::Visit(pgScene& scene) {
	//
}

void pgPass::Visit(pgSceneNode& node) {
	//
}

void pgPass::Visit(pgMesh& mesh) {
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
	m_pScene->_render(e);
}

void pgPassPilpeline::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	m_pPipeline->bind(e, flag);
}

void pgPassPilpeline::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	m_pPipeline->unbind(e, flag);
}


void pgPassPilpeline::update(pgRenderEventArgs& e) {
}

void pgPassPilpeline::Render() {
	if (m_pScene)
	{
		m_pScene->Accept(*this);
	}
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

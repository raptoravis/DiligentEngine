#include "../engine.h"

#include <windows.h>

namespace ade
{
void ReportErrorAndThrow(const std::string& file, int line, const std::string& function,
                         const std::string& message)
{
    std::stringstream ss;

    DWORD errorCode = GetLastError();
    LPTSTR errorMessage = nullptr;

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, 0, nullptr);

    if (errorMessage) {
        ss << file << "(" << line << "): "
           << "error " << errorCode << ": " << errorMessage << std::endl;
        LocalFree(errorMessage);
    } else {
        ss << file << "(" << line << "): " << message << std::endl;
    }

    OutputDebugStringA(ss.str().c_str());
    MessageBoxA(nullptr, message.c_str(), function.c_str(), MB_ICONERROR);
    throw new std::exception(message.c_str());
}
//////////////////////////////////////////////////////////////////////////
RenderEventArgs::RenderEventArgs() : pApp(0), pDeviceContext(0), pCamera(0) {}


void RenderEventArgs::set(float currentTime, float elapsedTime, App* caller, Camera* camera,
                            Diligent::RefCntAutoPtr<Diligent::IDeviceContext> ctx)
{
    pApp = caller;
    pDeviceContext = ctx;
    pCamera = camera;
    CurrTime = currentTime;
    ElapsedTime = elapsedTime;
}

uint32_t Object::s_uuid = 0;

Object::Object() : m_uuid(++Object::s_uuid) {}
//////////////////////////////////////////////////////////////////////////

Diligent::RefCntAutoPtr<Diligent::IRenderDevice> App::s_device;
Diligent::RefCntAutoPtr<Diligent::IDeviceContext> App::s_ctx;
Diligent::RefCntAutoPtr<Diligent::ISwapChain> App::s_swapChain;
Diligent::RefCntAutoPtr<Diligent::IEngineFactory> App::s_engineFactory;
std::shared_ptr<RenderTarget> App::s_rt;
std::shared_ptr<Texture> App::s_backBuffer;

Diligent::SwapChainDesc App::s_desc;
RenderEventArgs App::s_eventArgs;


App::App()
{
    //
}


App::~App()
{
    //
}

void App::Initialize(Diligent::IEngineFactory* pEngineFactory, Diligent::IRenderDevice* pDevice,
                       Diligent::IDeviceContext** ppContexts, Diligent::Uint32 NumDeferredCtx,
                       Diligent::ISwapChain* pSwapChain)
{
    //
}

void App::Render()
{
    //
}

void App::Update(double CurrTime, double ElapsedTime)
{
    //
}
}    // namespace ade
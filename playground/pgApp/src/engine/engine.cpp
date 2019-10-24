#include "engine.h"

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
pgRenderEventArgs::pgRenderEventArgs() : pApp(0), pDeviceContext(0), pCamera(0) {}


void pgRenderEventArgs::set(float currentTime, float elapsedTime, pgApp* caller, pgCamera* camera,
                            Diligent::RefCntAutoPtr<Diligent::IDeviceContext> ctx)
{
    pApp = caller;
    pDeviceContext = ctx;
    pCamera = camera;
    CurrTime = currentTime;
    ElapsedTime = elapsedTime;
}

uint32_t pgObject::s_uuid = 0;

pgObject::pgObject() : m_uuid(++pgObject::s_uuid) {}
//////////////////////////////////////////////////////////////////////////

Diligent::RefCntAutoPtr<Diligent::IRenderDevice> pgApp::s_device;
Diligent::RefCntAutoPtr<Diligent::IDeviceContext> pgApp::s_ctx;
Diligent::RefCntAutoPtr<Diligent::ISwapChain> pgApp::s_swapChain;
Diligent::RefCntAutoPtr<Diligent::IEngineFactory> pgApp::s_engineFactory;
std::shared_ptr<pgRenderTarget> pgApp::s_rt;
std::shared_ptr<pgTexture> pgApp::s_backBuffer;

Diligent::SwapChainDesc pgApp::s_desc;
pgRenderEventArgs pgApp::s_eventArgs;


pgApp::pgApp()
{
    //
}


pgApp::~pgApp()
{
    //
}

void pgApp::Initialize(Diligent::IEngineFactory* pEngineFactory, Diligent::IRenderDevice* pDevice,
                       Diligent::IDeviceContext** ppContexts, Diligent::Uint32 NumDeferredCtx,
                       Diligent::ISwapChain* pSwapChain)
{
    //
}

void pgApp::Render()
{
    //
}

void pgApp::Update(double CurrTime, double ElapsedTime)
{
    //
}
}    // namespace ade
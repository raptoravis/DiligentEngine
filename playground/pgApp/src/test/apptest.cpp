#include "apptest.h"

#include "test/technique/techniquedeferred.h"
#include "test/technique/techniqueforward.h"
#include "test/technique/techniqueforwardplus.h"
#include "test/technique/techniquegdr.h"
#include "test/technique/techniquetest.h"

#include "BasicMath.h"
#include "CommonlyUsedStates.h"
#include "FileSystem.h"
#include "GraphicsUtilities.h"
#include "MapHelper.h"
#include "ShaderMacroHelper.h"
#include "TextureUtilities.h"
#include "imGuIZMO.h"
#include "imgui.h"

#include <windows.h>

#include "engine/utils/mathutils.h"

#include "scenetest.h"

namespace Diligent
{
#include "BasicStructures.fxh"
//#include "ToneMappingStructures.fxh"

SampleBase* CreateSample()
{
    return new AppTest();
}
}    // namespace Diligent

// using namespace ade;

void AppTest::initLightData()
{
    ade::Light light1;

    light1.m_PositionWS = { -2.5f, -1.40531516f, 0.451306254f, 1.00000000f };
    light1.m_DirectionWS = { 0.0116977794f, -0.170993939f, -0.985219836f, 0.000000000f };
    light1.m_PositionVS = { 0, 0, 0, 1.f };
    light1.m_DirectionVS = { 0, 0, 0, 1.f };
    light1.m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    light1.m_SpotlightAngle = 36.2999992f;
    light1.m_Range = 12.f;
    light1.m_Intensity = 1.00000000f;
    light1.m_Enabled = 1;
    light1.m_Selected = 0;
    light1.m_Type = ade::Light::LightType::Point;

    m_Lights.push_back(light1);

    light1.m_PositionWS = { 2.5f, -1.40531516f, 0.451306254f, 1.00000000f };
    light1.m_DirectionWS = { 0.0116977794f, -0.170993939f, -0.985219836f, 0.000000000f };
    light1.m_PositionVS = { 0, 0, 0, 1.f };
    light1.m_DirectionVS = { 0, 0, 0, 1.f };
    light1.m_Color = { 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f };
    light1.m_SpotlightAngle = 36.2999992f;
    light1.m_Range = 12.f;
    light1.m_Intensity = 1.00000000f;
    light1.m_Enabled = 1;
    light1.m_Selected = 0;
    light1.m_Type = ade::Light::LightType::Spot;

    m_Lights.push_back(light1);

    ade::Light light2;

#if RIGHT_HANDED
    float dirz = -0.760157943f;
#else
    float dirz = 0.760157943f;
#endif

    light2.m_PositionWS = { -16.1441193f, 2.10133481f, 21.5686855f, 1.00000000f };
    light2.m_DirectionWS = { 0.585131526f, -0.282768548f, dirz, 0.000000000f };
    light2.m_PositionVS = { 0, 0, 0, 1.f };
    light2.m_DirectionVS = { 0, 0, 0, 1.f };
    light2.m_Color = { .4f, 0.4f, 0.4f, 1.0f };
    light2.m_SpotlightAngle = 45.f;
    light2.m_Range = 1.f;
    light2.m_Intensity = 1.00000000f;
    light2.m_Enabled = 1;
    light2.m_Selected = 0;
    light2.m_Type = ade::Light::LightType::Directional;

    m_Lights.push_back(light2);
}

void AppTest::initBuffers()
{
    m_PerObjectConstants =
        std::make_shared<ade::ConstantBuffer>((uint32_t)sizeof(ade::PassRender::PerObject));
    m_MaterialConstants =
        std::make_shared<ade::ConstantBuffer>((uint32_t)sizeof(ade::Material::MaterialProperties));
    m_LightsStructuredBuffer = std::make_shared<ade::StructuredBuffer>(
        m_Lights.data(), (uint32_t)m_Lights.size(), (uint32_t)sizeof(ade::Light),
        ade::CPUAccess::Write);
}


void AppTest::createRT()
{
    {
        // Create window-size offscreen render target
        TextureDesc RTColorDesc;
        RTColorDesc.Name = "RT Color";
        RTColorDesc.Type = RESOURCE_DIM_TEX_2D;
        RTColorDesc.Width = ade::App::s_desc.Width;
        RTColorDesc.Height = ade::App::s_desc.Height;
        RTColorDesc.MipLevels = 1;
        RTColorDesc.Format = ade::App::s_desc.ColorBufferFormat;
        // The render target can be bound as a shader resource and as a render target
        RTColorDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        // Define optimal clear value
        RTColorDesc.ClearValue.Format = RTColorDesc.Format;
        RTColorDesc.ClearValue.Color[0] = 0.f;
        RTColorDesc.ClearValue.Color[1] = 0.f;
        RTColorDesc.ClearValue.Color[2] = 0.f;
        RTColorDesc.ClearValue.Color[3] = 1.f;

        RefCntAutoPtr<ITexture> colorTextureI;
        ade::App::s_device->CreateTexture(RTColorDesc, nullptr, &colorTextureI);

        std::shared_ptr<ade::Texture> colorTexture = std::make_shared<ade::Texture>(colorTextureI);

        // Create depth buffer
        TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "RT depth stencil";
        DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = ade::App::s_desc.Width;
        DepthBufferDesc.Height = ade::App::s_desc.Height;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        // DepthBufferDesc.Format = ade::App::s_desc.DepthBufferFormat;
        DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
        DepthBufferDesc.SampleCount = ade::App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
        DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

        RefCntAutoPtr<ITexture> depthStencilTextureI;
        ade::App::s_device->CreateTexture(DepthBufferDesc, nullptr, &depthStencilTextureI);

        std::shared_ptr<ade::Texture> depthStencilTexture =
            std::make_shared<ade::Texture>(depthStencilTextureI);

        //
        m_pRenderTarget = std::make_shared<ade::RenderTarget>();

        m_pRenderTarget->AttachTexture(ade::RenderTarget::AttachmentPoint::Color0, colorTexture);
        m_pRenderTarget->AttachTexture(ade::RenderTarget::AttachmentPoint::DepthStencil,
                                       depthStencilTexture);
    }

    //
    {
        auto colorTextureI = m_pSwapChain->GetCurrentBackBufferRTV()->GetTexture();
        auto depthStencilTextureI = m_pSwapChain->GetDepthBufferDSV()->GetTexture();

        m_pBackBuffer = std::make_shared<ade::Texture>(colorTextureI);
        m_pDepthStencilBuffer = std::make_shared<ade::Texture>(depthStencilTextureI);
    }
}

void AppTest::Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice,
                         IDeviceContext** ppContexts, Uint32 NumDeferredCtx, ISwapChain* pSwapChain)
{
    SampleBase::Initialize(pEngineFactory, pDevice, ppContexts, NumDeferredCtx, pSwapChain);

    // _CrtSetBreakAlloc(38791);

    //
    ade::App::s_device.Attach(pDevice);
    ade::App::s_ctx.Attach(*ppContexts);
    ade::App::s_swapChain.Attach(pSwapChain);
    ade::App::s_engineFactory.Attach(pEngineFactory);
    ade::App::s_desc = pSwapChain->GetDesc();

    createRT();

    m_renderingTechnique = RenderingTechnique::Gdr;
    // m_renderingTechnique = RenderingTechnique::ForwardPlus;
    // m_renderingTechnique = RenderingTechnique::Deferred;
    // m_renderingTechnique = RenderingTechnique::Forward;
    // m_renderingTechnique = RenderingTechnique::Test;

    Diligent::float3 pos = Diligent::float3(0, 0, 0);
    Diligent::float3 dir = Diligent::float3(0, 0, -1);
    if (m_renderingTechnique != RenderingTechnique::Test) {
#if RIGHT_HANDED
        float z = 25;
        dir = Diligent::float3(0, 0, -1);
#else
        float z = -25;
        // inverted by camera
        dir = Diligent::float3(0, 0, -1);
#endif
        pos = float3(0, 0, z);
    } else {
#if RIGHT_HANDED
        dir = Diligent::float3(0, 0, 1);
#else
        // inverted by camera
        dir = Diligent::float3(0, 0, -1);
#endif
    }

    m_pCamera = std::make_shared<ade::Camera>(pos, dir);

    // technique will clean up passed added in it
    m_pTechnique = std::make_shared<ade::TechniqueTest>(m_pRenderTarget, m_pBackBuffer);

    m_pForwardTechnique = std::make_shared<ade::TechniqueForward>(m_pRenderTarget, m_pBackBuffer);
    m_pDeferredTechnique = std::make_shared<ade::TechniqueDeferred>(m_pRenderTarget, m_pBackBuffer);
    m_pForwardPlusTechnique =
        std::make_shared<ade::TechniqueForwardPlus>(m_pRenderTarget, m_pBackBuffer);
    m_pGdrTechnique = std::make_shared<ade::TechniqueGdr>(m_pRenderTarget, m_pBackBuffer);

    //
    std::shared_ptr<SceneTest> testScene = std::make_shared<SceneTest>();
    std::wstring filePath = L"resources/models/test/test_scene.nff";
    testScene->LoadFromFile(filePath);
    testScene->customMesh();

    initLightData();
    initBuffers();

    // if (m_renderingTechnique == RenderingTechnique::Forward)
    {
        auto forwardTech = (ade::TechniqueForward*)m_pForwardTechnique.get();
        forwardTech->Set(ade::PassRender::kPerObjectName, m_PerObjectConstants);
        forwardTech->Set(ade::PassRender::kMaterialName, m_MaterialConstants);
        forwardTech->Set(ade::PassRender::kLightsName, m_LightsStructuredBuffer);

        forwardTech->init(testScene, &m_Lights);
    }

    // if (m_renderingTechnique == RenderingTechnique::Deferred)
    {
        auto deferredTech = (ade::TechniqueDeferred*)m_pDeferredTechnique.get();

        deferredTech->Set(ade::PassRender::kPerObjectName, m_PerObjectConstants);
        deferredTech->Set(ade::PassRender::kMaterialName, m_MaterialConstants);
        deferredTech->Set(ade::PassRender::kLightsName, m_LightsStructuredBuffer);

        deferredTech->init(testScene, &m_Lights);
    }

    // if (m_renderingTechnique == RenderingTechnique::ForwardPlus)
    {
        auto fpTech = (ade::TechniqueForwardPlus*)m_pForwardPlusTechnique.get();

        fpTech->Set(ade::PassRender::kPerObjectName, m_PerObjectConstants);
        fpTech->Set(ade::PassRender::kMaterialName, m_MaterialConstants);
        fpTech->Set(ade::PassRender::kLightsName, m_LightsStructuredBuffer);

        fpTech->init(testScene, &m_Lights, m_pCamera);
    }

    {
        auto gdrTech = (ade::TechniqueGdr*)m_pGdrTechnique.get();

        gdrTech->init();
    }
}

AppTest::~AppTest()
{
    //
    ade::App::s_device.Detach();
    ade::App::s_ctx.Detach();
    ade::App::s_swapChain.Detach();
    ade::App::s_engineFactory.Detach();
    // ade::App::s_desc = pSwapChain->GetDesc();
}

// Render a frame
void AppTest::Render()
{
    //// Clear the back buffer
    // const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
    // ade::App::s_ctx->ClearRenderTarget(nullptr, ClearColor,
    // RESOURCE_STATE_TRANSITION_MODE_TRANSITION); ade::App::s_ctx->ClearDepthStencil(nullptr,
    // CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    if (m_renderingTechnique == RenderingTechnique::Test) {
        m_pTechnique->Render();
    }

    if (m_renderingTechnique == RenderingTechnique::Forward) {
        m_pForwardTechnique->Render();
    }

    if (m_renderingTechnique == RenderingTechnique::Deferred) {
        m_pDeferredTechnique->Render();
    }

    if (m_renderingTechnique == RenderingTechnique::ForwardPlus) {
        m_pForwardPlusTechnique->Render();
    }

    if (m_renderingTechnique == RenderingTechnique::Gdr) {
        m_pGdrTechnique->Render();
    }

    // auto srcTexture = m_pRenderTarget->GetTexture(ade::RenderTarget::AttachmentPoint::Color0);
    // srcTexture->Copy(m_pBackBuffer.get());
}


void AppTest::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_pCamera->update(&m_InputController, (float)ElapsedTime);

    ade::App::s_eventArgs.set((float)CurrTime, (float)ElapsedTime, this, m_pCamera.get(),
                              ade::App::s_ctx);

    int technique = (int)m_renderingTechnique;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* desc[] = { "test", "forward", "deferred", "forward+", "gdr", "lightprepass" };
        // ImGui::SetNextItemWidth(100);
        ImGui::Combo("technique", &technique, desc, IM_ARRAYSIZE(desc));

        // Quaternion rot = Quaternion(m_viewMatrix);
        // ImGui::gizmo3D("Model Rotation", rot, ImGui::GetTextLineHeight() * 10);
        const float3& pos = m_pCamera->getPos();
        float3 look = m_pCamera->getLook();

        ImGui::Text("pos: %f %f %f", pos.x, pos.y, pos.z);

        ImGui::Text("look: %f %f %f", look.x, look.y, look.z);

        if (ImGui::Button("Reset view")) {
            m_pCamera->reset();
        }

        ImGui::Separator();

        float4x4 camTt = m_pCamera->getViewMatrix();

        // Quaternion rot = ade::mRot2Quat(camTt);
        Quaternion rot = ade::calculateRotation(camTt);

        ImGui::gizmo3D("Camera", rot, ImGui::GetTextLineHeight() * 10);

        if (m_renderingTechnique == RenderingTechnique::Test) {
            m_pTechnique->Update();
        }

        if (m_renderingTechnique == RenderingTechnique::Forward) {
            m_pForwardTechnique->Update();
        }

        if (m_renderingTechnique == RenderingTechnique::Deferred) {
            m_pDeferredTechnique->Update();
        }

        if (m_renderingTechnique == RenderingTechnique::ForwardPlus) {
            m_pForwardPlusTechnique->Update();
        }

        if (m_renderingTechnique == RenderingTechnique::Gdr) {
            m_pGdrTechnique->Update();
        }
    }

    if ((int)m_renderingTechnique != technique) {
        m_renderingTechnique = (RenderingTechnique)technique;

        if (m_renderingTechnique == RenderingTechnique::Test) {
#if RIGHT_HANDED
            float3 dir = Diligent::float3(0, 0, 1);
#else
            // inverted by camera
            float3 dir = Diligent::float3(0, 0, -1);
#endif

            m_pCamera->reset(float3(0, 0, 0), dir);
        } else {
#if RIGHT_HANDED
            float3 dir = Diligent::float3(0, 0, -1);
            float z = 25.0f;
#else
            // inverted by camera
            float3 dir = Diligent::float3(0, 0, -1);
            float z = -25.0f;
#endif

            m_pCamera->reset(float3(0, 0, z), dir);
        }
    }

    ImGui::End();
}

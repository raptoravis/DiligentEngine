#include "apptest.h"

#include "engine/technique/techniquedeferred.h"
#include "engine/technique/techniqueforward.h"
#include "engine/technique/techniqueforwardplus.h"
#include "engine/technique/techniquetest.h"

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

void AppTest::initLightData()
{
    pgLight light1;

    light1.m_PositionVS = { -0.535466492f, -1.40531516f, 0.451306254f, 1.00000000f };
    light1.m_DirectionWS = { 0.0116977794f, -0.170993939f, -0.985219836f, 0.000000000f };
    light1.m_PositionVS = { -0.00858783722f, -0.924078941f, -32.9505081f, 1.00000000f };
    light1.m_DirectionVS = { 0.0165561363f, -0.176791593f, -0.984109104f, 0.000000000f };
    light1.m_Color = { 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f };
    light1.m_SpotlightAngle = 36.2999992f;
    light1.m_Range = 16.5599995f;
    light1.m_Intensity = 1.00000000f;
    light1.m_Enabled = 1;
    light1.m_Selected = 0;
    light1.m_Type = pgLight::LightType::Point;

    pgLight light2;

    light2.m_PositionVS = { -16.1441193f, 2.10133481f, 21.5686855f, 1.00000000f };
    light2.m_DirectionWS = { 0.585131526f, -0.282768548f, -0.760157943f, 0.000000000f };
    light2.m_PositionVS = { -15.7199173f, 2.60687327f, -11.9134722f, 1.00000000f };
    light2.m_DirectionVS = { 0.589790821f, -0.283460617f, -0.756172597f, 0.000000000f };
    light2.m_Color = { 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f };
    light2.m_SpotlightAngle = 45.f;
    light2.m_Range = 1.f;
    light2.m_Intensity = 1.00000000f;
    light2.m_Enabled = 1;
    light2.m_Selected = 0;
    light2.m_Type = pgLight::LightType::Directional;

    m_Lights.push_back(light1);
    m_Lights.push_back(light2);
}

void AppTest::initBuffers()
{
    m_PerObjectConstants =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(pgPassRender::PerObject));
    m_MaterialConstants =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(pgMaterial::MaterialProperties));
    m_LightsStructuredBuffer =
        std::make_shared<StructuredBuffer>(m_Lights.data(), (uint32_t)m_Lights.size(),
                                           (uint32_t)sizeof(pgLight), CPUAccess::None, true);
}


void AppTest::createRT()
{
    {
        // Create window-size offscreen render target
        TextureDesc RTColorDesc;
        RTColorDesc.Name = "RT Color";
        RTColorDesc.Type = RESOURCE_DIM_TEX_2D;
        RTColorDesc.Width = pgApp::s_desc.Width;
        RTColorDesc.Height = pgApp::s_desc.Height;
        RTColorDesc.MipLevels = 1;
        RTColorDesc.Format = pgApp::s_desc.ColorBufferFormat;
        // The render target can be bound as a shader resource and as a render target
        RTColorDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        // Define optimal clear value
        RTColorDesc.ClearValue.Format = RTColorDesc.Format;
        RTColorDesc.ClearValue.Color[0] = 0.f;
        RTColorDesc.ClearValue.Color[1] = 0.f;
        RTColorDesc.ClearValue.Color[2] = 0.f;
        RTColorDesc.ClearValue.Color[3] = 1.f;

        RefCntAutoPtr<ITexture> colorTextureI;
        pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &colorTextureI);

        std::shared_ptr<pgTexture> colorTexture = std::make_shared<pgTexture>(colorTextureI);

        // Create depth buffer
        TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "RT depth stencil";
        DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = pgApp::s_desc.Width;
        DepthBufferDesc.Height = pgApp::s_desc.Height;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        // DepthBufferDesc.Format = pgApp::s_desc.DepthBufferFormat;
        DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
        DepthBufferDesc.SampleCount = pgApp::s_desc.SamplesCount;
        DepthBufferDesc.Usage = USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
        DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

        RefCntAutoPtr<ITexture> depthStencilTextureI;
        pgApp::s_device->CreateTexture(DepthBufferDesc, nullptr, &depthStencilTextureI);

        std::shared_ptr<pgTexture> depthStencilTexture =
            std::make_shared<pgTexture>(depthStencilTextureI);

        //
        m_pRT = std::make_shared<pgRenderTarget>();

        m_pRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color0, colorTexture);
        m_pRT->AttachTexture(pgRenderTarget::AttachmentPoint::DepthStencil, depthStencilTexture);
    }

    //
    {
        auto colorTextureI = m_pSwapChain->GetCurrentBackBufferRTV()->GetTexture();
        auto depthStencilTextureI = m_pSwapChain->GetDepthBufferDSV()->GetTexture();

        m_pBackBuffer = std::make_shared<pgTexture>(colorTextureI);
        m_pDepthStencilBuffer = std::make_shared<pgTexture>(depthStencilTextureI);
    }
}

void AppTest::Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice,
                         IDeviceContext** ppContexts, Uint32 NumDeferredCtx, ISwapChain* pSwapChain)
{
    SampleBase::Initialize(pEngineFactory, pDevice, ppContexts, NumDeferredCtx, pSwapChain);

    //
    pgApp::s_device.Attach(pDevice);
    pgApp::s_ctx.Attach(*ppContexts);
    pgApp::s_swapChain.Attach(pSwapChain);
    pgApp::s_engineFactory.Attach(pEngineFactory);
    pgApp::s_desc = pSwapChain->GetDesc();

    createRT();

    // m_renderingTechnique = RenderingTechnique::ForwardPlus;
    m_renderingTechnique = RenderingTechnique::Deferred;
    // m_renderingTechnique = RenderingTechnique::Forward;
    // m_renderingTechnique = RenderingTechnique::Test;

    Diligent::float3 pos = Diligent::float3(0, 0, 0);
    if (m_renderingTechnique != RenderingTechnique::Test) {
        pos = float3(0, 0, -25);
    }

    m_pCamera = std::make_shared<pgCamera>(pos, Diligent::float3(0, 0, -1));

    // technique will clean up passed added in it
    m_pTechnique = std::make_shared<TechniqueTest>(m_pRT, m_pBackBuffer);

    m_pForwardTechnique = std::make_shared<TechniqueForward>(m_pRT, m_pBackBuffer);
    m_pDeferredTechnique = std::make_shared<TechniqueDeferred>(m_pRT, m_pBackBuffer);
    m_pForwardPlusTechnique = std::make_shared<TechniqueForwardPlus>(m_pRT, m_pBackBuffer);

    //
    std::shared_ptr<SceneTest> testScene = std::make_shared<SceneTest>();
    std::wstring filePath = L"resources/models/test/test_scene.nff";
    testScene->LoadFromFile(filePath);
    testScene->customMesh();

    initLightData();
    initBuffers();

    // if (m_renderingTechnique == RenderingTechnique::Forward)
    {
        auto forwardTech = (TechniqueForward*)m_pForwardTechnique.get();
        forwardTech->init(testScene, m_Lights);

        forwardTech->SetResource("PerObject", m_PerObjectConstants);
        forwardTech->SetResource("Material", m_MaterialConstants);
        forwardTech->SetResource("Lights", m_LightsStructuredBuffer);
    }

    // if (m_renderingTechnique == RenderingTechnique::Deferred)
    {
        auto deferredTech = (TechniqueDeferred*)m_pDeferredTechnique.get();
        deferredTech->init(testScene, m_Lights);

        deferredTech->SetResource("PerObject", m_PerObjectConstants);
        deferredTech->SetResource("Material", m_MaterialConstants);
        deferredTech->SetResource("Lights", m_LightsStructuredBuffer);
    }

    if (m_renderingTechnique == RenderingTechnique::ForwardPlus) {
        auto fpTech = (TechniqueForwardPlus*)m_pForwardPlusTechnique.get();
        fpTech->init(testScene, m_Lights);

        fpTech->SetResource("PerObject", m_PerObjectConstants);
        fpTech->SetResource("Material", m_MaterialConstants);
        fpTech->SetResource("Lights", m_LightsStructuredBuffer);
    }
}

AppTest::~AppTest()
{
    //
    pgApp::s_device.Detach();
    pgApp::s_ctx.Detach();
    pgApp::s_swapChain.Detach();
    pgApp::s_engineFactory.Detach();
    // pgApp::s_desc = pSwapChain->GetDesc();
}

// Render a frame
void AppTest::Render()
{
    //// Clear the back buffer
    // const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
    // pgApp::s_ctx->ClearRenderTarget(nullptr, ClearColor,
    // RESOURCE_STATE_TRANSITION_MODE_TRANSITION); pgApp::s_ctx->ClearDepthStencil(nullptr,
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

    // auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    // srcTexture->Copy(m_pBackBuffer.get());
}


void AppTest::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_pCamera->update(&m_InputController, (float)ElapsedTime);

    pgApp::s_eventArgs.set((float)CurrTime, (float)ElapsedTime, this, m_pCamera.get(),
                           pgApp::s_ctx);

    int technique = (int)m_renderingTechnique;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* desc[] = { "test", "forward", "deferred", "forward+" };
        // ImGui::SetNextItemWidth(100);
        ImGui::Combo("technique", &technique, desc, IM_ARRAYSIZE(desc));

        // Quaternion rot = Quaternion(m_viewMatrix);
        // ImGui::gizmo3D("Model Rotation", rot, ImGui::GetTextLineHeight() * 10);
        const float3& pos = m_pCamera->getPos();
        const float3& look = m_pCamera->getLook();

        ImGui::Text("pos: %f %f %f", pos.x, pos.y, pos.z);

        // we need to negative it
        ImGui::Text("look: %f %f %f", -look.x, -look.y, -look.z);

        if (ImGui::Button("Reset view")) {
            m_pCamera->reset();
        }

        ImGui::Separator();

        float4x4 camTt = m_pCamera->getViewMatrix();

        // Quaternion rot = mRot2Quat(camTt);
        Quaternion rot = calculateRotation(camTt);

        ImGui::gizmo3D("Camera", rot, ImGui::GetTextLineHeight() * 10);
    }

    if ((int)m_renderingTechnique != technique) {
        m_renderingTechnique = (RenderingTechnique)technique;

        if (m_renderingTechnique == RenderingTechnique::Test) {
            m_pCamera->reset(float3(0, 0, 0), float3(0, 0, -1));
        } else {
            m_pCamera->reset(float3(0, 0, -25), float3(0, 0, -1));
        }
    }

    ImGui::End();
}
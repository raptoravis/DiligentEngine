#include "techniquegdr.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"

#include "engine/utils/mathutils.h"

#include "../mesh/meshprop.h"

#include "../pass/passgdr.h"
#include "engine/pass/passinvokefunction.h"

#include "engine/pass/passdispatch.h"
#include "engine/pipeline/pipelinedispatch.h"

#include "../scene/scenegdr.h"


using namespace ade;

static const uint16_t s_maxNoofProps = 10;
static const uint16_t s_maxNoofInstances = 2048;

#define RENDER_PASS_HIZ_ID 0
#define RENDER_PASS_HIZ_DOWNSCALE_ID 1
#define RENDER_PASS_OCCLUDE_PROPS_ID 2
#define RENDER_PASS_COMPACT_STREAM_ID 3
#define RENDER_PASS_MAIN_ID 4

uint8_t calcNumMips(uint32_t _width, uint32_t _height, uint32_t _depth = 1)
{
    const uint32_t max = ade::max(_width, _height, _depth);
    const uint32_t num = 1 + uint32_t(ade::log2(float(max)));

    return uint8_t(num);
}


TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
    m_pSceneGdr = std::make_shared<SceneGdr>();
    m_pSceneGdr->create();
}

void TechniqueGdr::initDebug()
{
    {
        std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
        AddPass(pSetRTPass);

        std::shared_ptr<PassClearRT> pClearRTPass =
            std::make_shared<PassClearRT>(this, m_pRenderTarget);
        AddPass(pClearRTPass);

        {
            //////////////////////////////////////////////////////////////////////////
            m_PerObject = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::PerObject));
            m_materialId = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::MaterialId));
            u_colors = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::Colors));

            this->Set(PassGdr::kPerObjectName, m_PerObject);
            this->Set(PassGdr::kMaterialIdName, m_materialId);
            this->Set(PassGdr::kColorsName, u_colors);

            {
                PassGdr::Colors colors;

                uint32_t noofMaterials = m_pSceneGdr->m_noofMaterials;
                noofMaterials = std::min(noofMaterials, 32u);
                memcpy(colors.colors, m_pSceneGdr->m_materials,
                       sizeof(m_pSceneGdr->m_materials[0]) * noofMaterials);
                SetColorsConstantBufferData(colors);
            }

            {
                auto prop0 = m_pSceneGdr->m_props[0];
                std::shared_ptr<MeshProp> meshProp = std::make_shared<MeshProp>(&prop0);

                auto scene = std::make_shared<Scene>();
                float4x4 trans1 = float4x4::Identity();
                std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
                root1->AddMesh(meshProp);
                scene->SetRootNode(root1);

                //////////////////////////////////////////////////////////////////////////
                std::shared_ptr<Pass> pPass = createPassGdr(scene);

                AddPass(pPass);
            }
            {
                auto prop0 = m_pSceneGdr->m_props[1];
                std::shared_ptr<MeshProp> meshProp = std::make_shared<MeshProp>(&prop0);

                auto scene = std::make_shared<Scene>();
                float4x4 trans1 = float4x4::Identity();
                std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
                root1->AddMesh(meshProp);
                scene->SetRootNode(root1);

                //////////////////////////////////////////////////////////////////////////
                std::shared_ptr<Pass> pPass = createPassGdr(scene);

                AddPass(pPass);
            }
            {
                auto prop0 = m_pSceneGdr->m_props[2];
                std::shared_ptr<MeshProp> meshProp = std::make_shared<MeshProp>(&prop0);

                auto scene = std::make_shared<Scene>();
                float4x4 trans1 = float4x4::Identity();
                std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
                root1->AddMesh(meshProp);
                scene->SetRootNode(root1);

                //////////////////////////////////////////////////////////////////////////
                std::shared_ptr<Pass> pPass = createPassGdr(scene);

                AddPass(pPass);
            }
            {
                auto prop0 = m_pSceneGdr->m_props[3];
                std::shared_ptr<MeshProp> meshProp = std::make_shared<MeshProp>(&prop0);

                auto scene = std::make_shared<Scene>();
                float4x4 trans1 = float4x4::Identity();
                std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>(trans1);
                root1->AddMesh(meshProp);
                scene->SetRootNode(root1);

                //////////////////////////////////////////////////////////////////////////
                std::shared_ptr<Pass> pPass = createPassGdr(scene);

                AddPass(pPass);
            }
        }

        //
        {
            auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
            auto dstTexture = m_pBackBuffer;

            std::shared_ptr<PassCopyTexture> pCopyTexPass =
                std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
            AddPass(pCopyTexPass);
        }
    }
}

void TechniqueGdr::SetColorsConstantBufferData(PassGdr::Colors& data)
{
    auto cb = std::dynamic_pointer_cast<ConstantBuffer>(this->Get(PassGdr::kColorsName));

    cb->Set(data);
}


std::shared_ptr<ade::Pass> TechniqueGdr::createPassGdr(std::shared_ptr<ade::Scene> scene)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(m_pRenderTarget);

    {
        std::shared_ptr<Shader> vs = std::make_shared<ade::Shader>();
        vs->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                               "vs_gdr_instanced_indirect_rendering.vsh", "main", "./gdr", false);

        std::shared_ptr<Shader> ps = std::make_shared<ade::Shader>();
        ps->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                               "fs_gdr_instanced_indirect_rendering.psh", "main", "./gdr", false);

        vs->GetShaderParameterByName(PassGdr::kPerObjectName).Set(m_PerObject);
        vs->GetShaderParameterByName(PassGdr::kMaterialIdName).Set(m_materialId);

        ps->GetShaderParameterByName(PassGdr::kColorsName).Set(u_colors);

        pipeline->SetShader(ade::Shader::Shader::VertexShader, vs);
        pipeline->SetShader(ade::Shader::Shader::PixelShader, ps);

        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
            // LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
            //               sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
        };

        pipeline->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    std::shared_ptr<PassGdr> pPass = std::make_shared<PassGdr>(this, scene, pipeline);

    return pPass;
}


TechniqueGdr::~TechniqueGdr()
{
    //
}

void TechniqueGdr::Render()
{
    base::Render();
}

void TechniqueGdr::Update()
{
    ImGui::Separator();

    bool bDebug = m_bDebug;
    ImGui::Checkbox("debug", &m_bDebug);
    ImGui::Separator();

    if (bDebug != m_bDebug) {
        init();
    }
}

void TechniqueGdr::createHiZBuffers()
{
    {
        Diligent::TEXTURE_FORMAT dsvFormat =
            TEX_FORMAT_D24_UNORM_S8_UINT;    // TEX_FORMAT_R24G8_TYPELESS;

        // Create depth buffer
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZDepthBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = m_hiZwidth;
        DepthBufferDesc.Height = m_hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = dsvFormat;
        DepthBufferDesc.SampleCount = 1;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        m_hiZDepthBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);
    }

    // how many mip will the Hi Z buffer have?
    m_noofHiZMips = (uint32_t)calcNumMips(m_hiZwidth, m_hiZheight);
    uint32_t hiZwidth = m_hiZwidth;
    uint32_t hiZheight = m_hiZheight;

    for (uint32_t i = 0; i < m_noofHiZMips; ++i) {
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = hiZwidth;
        DepthBufferDesc.Height = hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_R32_FLOAT;
        DepthBufferDesc.SampleCount = 1;    // App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        auto hiZBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);

        m_hiZBuffers.push_back(hiZBuffer);
        // m_hiZBuffer = Scene::CreateTexture2D((uint16_t)m_hiZwidth, (uint16_t)m_hiZheight, 1,
        //                                     Diligent::TEX_FORMAT_R32_FLOAT, CPUAccess::None,
        //                                     true);

        hiZwidth = ade::max(hiZwidth / 2, 1u);
        hiZheight = ade::max(hiZheight / 2, 1u);
    }

    // Create uniforms and samplers.
    // u_inputRTSize = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
    u_cullingConfig = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));

    // SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
    //                                 FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
    //                                 TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };

    // StaticSamplerDesc samplerDesc{ SHADER_TYPE_PIXEL, "s_texOcclusionDepth", linearRepeatSampler
    // };

    // s_texOcclusionDepth = std::make_shared<SamplerState>(samplerDesc);

    {
        // The compute shader will write how many unoccluded instances per drawcall there are here
        m_drawcallInstanceCounts = ade::Scene::CreateFormatBuffer(
            ade::App::s_device, nullptr, Diligent::VALUE_TYPE::VT_UINT32, s_maxNoofProps,
            sizeof(uint32_t), false, true);

        // the compute shader will write the result of the occlusion test for each instance here
        m_instancePredicates = ade::Scene::CreateFormatBuffer(
            ade::App::s_device, nullptr, Diligent::VALUE_TYPE::VT_UINT32, s_maxNoofInstances,
            sizeof(uint32_t), true, true);
    }

    // bounding box for each instance, will be fed to the compute shader to calculate occlusion
    {
        // initialise the buffer with the bounding boxes of all instances
        const int sizeOfBuffer = 2 * 4 * m_pSceneGdr->m_totalInstancesCount;
        float* boundingBoxes = new float[sizeOfBuffer];

        float* data = boundingBoxes;
        for (uint16_t i = 0; i < m_pSceneGdr->m_noofProps; i++) {
            Prop& prop = m_pSceneGdr->m_props[i];

            const uint32_t numInstances = prop.m_noofInstances;

            for (uint32_t j = 0; j < numInstances; j++) {
                ade::memCopy(data, &prop.m_instances[j].m_bboxMin, 3 * sizeof(float));
                data[3] =
                    (float)i;    // store the drawcall ID here to avoid creating a separate buffer
                data += 4;

                ade::memCopy(data, &prop.m_instances[j].m_bboxMax, 3 * sizeof(float));
                data += 4;
            }
        }

        // bSRV
        m_instanceBoundingBoxes = std::make_shared<ade::StructuredBuffer>(
            boundingBoxes, m_pSceneGdr->m_totalInstancesCount * 2,
            (uint32_t)sizeof(Diligent::float4), CPUAccess::None, false);
    }

    // pre and post occlusion culling instance data buffers
    {
        // initialise the buffer with data for all instances
        // Currently we only store a world matrix (16 floats)
        const int sizeOfBuffer = 16 * m_pSceneGdr->m_totalInstancesCount;
        float* instanceData = new float[sizeOfBuffer];

        float* data = instanceData;
        for (uint16_t ii = 0; ii < m_pSceneGdr->m_noofProps; ++ii) {
            Prop& prop = m_pSceneGdr->m_props[ii];

            const uint32_t numInstances = prop.m_noofInstances;

            for (uint32_t jj = 0; jj < numInstances; ++jj) {
                ade::memCopy(data, &prop.m_instances[jj].m_world, 16 * sizeof(float));
                data[3] =
                    float(ii);    // store the drawcall ID here to avoid creating a separate buffer
                data += 16;
            }
        }

        // pre occlusion buffer
        m_instanceBuffer = std::make_shared<ade::StructuredBuffer>(
            instanceData, m_pSceneGdr->m_totalInstancesCount, (uint32_t)sizeof(Diligent::float4),
            CPUAccess::None, false);

        // post occlusion buffer
        m_culledInstanceBuffer = std::make_shared<ade::StructuredBuffer>(
            nullptr, m_pSceneGdr->m_totalInstancesCount, (uint32_t)sizeof(Diligent::float4),
            CPUAccess::None, true);
    }

    // we use one "drawcall" per prop to render all its instances
    m_indirectBuffer = std::make_shared<ade::StructuredBuffer>(nullptr, m_pSceneGdr->m_noofProps,
                                                               (uint32_t)sizeof(Diligent::uint4),
                                                               CPUAccess::None, true);

    //////////////////////////////////////////////////////////////////////////
    // Calculate how many vertices/indices the master buffers will need.
    uint16_t totalNoofVertices = 0;
    uint16_t totalNoofIndices = 0;
    for (uint16_t i = 0; i < m_pSceneGdr->m_noofProps; i++) {
        Prop& prop = m_pSceneGdr->m_props[i];

        totalNoofVertices += prop.m_noofVertices;
        totalNoofIndices += prop.m_noofIndices;
    }

    // CPU data to fill the master buffers
    m_allPropVerticesDataCPU = new PosVertex[totalNoofVertices];
    m_allPropIndicesDataCPU = new uint32_t[totalNoofIndices];
    m_indirectBufferDataCPU = new uint32_t[m_pSceneGdr->m_noofProps * 3];

    // Copy data over to the master buffers
    PosVertex* propVerticesData = m_allPropVerticesDataCPU;
    uint32_t* propIndicesData = m_allPropIndicesDataCPU;

    uint16_t vertexBufferOffset = 0;
    uint16_t indexBufferOffset = 0;

    for (uint16_t i = 0; i < m_pSceneGdr->m_noofProps; i++) {
        Prop& prop = m_pSceneGdr->m_props[i];

        ade::memCopy(propVerticesData, prop.m_vertices, prop.m_noofVertices * sizeof(PosVertex));
        ade::memCopy(propIndicesData, prop.m_indices, prop.m_noofIndices * sizeof(uint16_t));

        propVerticesData += prop.m_noofVertices;
        propIndicesData += prop.m_noofIndices;

        m_indirectBufferDataCPU[i * 3] = prop.m_noofIndices;
        m_indirectBufferDataCPU[i * 3 + 1] = indexBufferOffset;
        m_indirectBufferDataCPU[i * 3 + 2] = vertexBufferOffset;

        indexBufferOffset += prop.m_noofIndices;
        vertexBufferOffset += prop.m_noofVertices;
    }

    // Create master vertex buffer
    m_allPropsVertexbufferHandle = Scene::CreateFloatVertexBuffer(
        App::s_device, (float*)m_allPropVerticesDataCPU, totalNoofVertices, 3 * sizeof(PosVertex));

    // Create master index buffer.
    m_allPropsIndexbufferHandle = Scene::CreateUIntIndexBuffer(
        App::s_device, m_allPropIndicesDataCPU, totalNoofIndices * sizeof(uint32_t));

    // Create buffer with const drawcall data which will be copied to the indirect buffer later.
    m_indirectBufferData = Scene::CreateFormatBuffer(
        App::s_device, m_indirectBufferDataCPU, Diligent::VALUE_TYPE::VT_UINT32,
        m_pSceneGdr->m_noofProps * 3, sizeof(uint32_t), true);

    m_useIndirect = true;
    m_firstFrame = true;
}

std::shared_ptr<ade::Shader> TechniqueGdr::loadProgram(const std::string& shader,
                                                       ade::Shader::ShaderType st)
{
    std::shared_ptr<Shader> sh = std::make_shared<ade::Shader>();
    sh->LoadShaderFromFile(st, shader.c_str(), "main", "./gdr", false);

    return sh;
}

static void SetVertexBuffer(uint32_t slot, std::shared_ptr<Buffer> pBuffer)
{
    auto buffer = pBuffer->GetBuffer();

    Diligent::Uint32 offset[] = { 0 };
    Diligent::IBuffer* pBuffs[] = { buffer };
    const uint32_t buffs = 1;

    App::s_ctx->SetVertexBuffers(slot, buffs, pBuffs, offset,
                                 Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                 Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
}

static void SetIndexBuffer(std::shared_ptr<Buffer> pBuffer)
{
    App::s_ctx->SetIndexBuffer(pBuffer->GetBuffer(), 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

static void SetInstanceBuffer(uint32_t slot, std::shared_ptr<Buffer> pBuffer)
{
    SetVertexBuffer(slot, pBuffer);
}

static void Submit(std::shared_ptr<Buffer> pBuffer, uint32_t instancesCount)
{
    SetIndexBuffer(pBuffer);

    {
        auto count = pBuffer->GetCount();

        DrawIndexedAttribs DrawAttrs;
        DrawAttrs.IndexType = VT_UINT32;    // Index type
        DrawAttrs.NumIndices = count;
        DrawAttrs.NumInstances = instancesCount;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

        App::s_ctx->DrawIndexed(DrawAttrs);
    }
}

// renders the occluders to a depth buffer
void TechniqueGdr::renderOcclusionBufferPass()
{
    std::shared_ptr<ade::RenderTarget> renderTarget = std::make_shared<ade::RenderTarget>();
    renderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_hiZDepthBuffer);
    // renderTarget->AttachTexture(
    //    RenderTarget::AttachmentPoint::DepthStencil,
    //    m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil));


    std::shared_ptr<ade::Shader> shaderOccusionPass =
        loadProgram("vs_gdr_render_occlusion.sh", ade::Shader::VertexShader);
    m_pipelineOccusionPass = std::make_shared<Pipeline>(renderTarget);

    // Diligent::TEXTURE_FORMAT RTFormat = Diligent::TEX_FORMAT_UNKNOWN;
    // Diligent::TEXTURE_FORMAT DSFormat = m_hiZDepthBuffer->GetTexture()->GetDesc().Format;
    // m_pipelineOccusionPass->SetRenderTargetFormat(RTFormat, DSFormat);

    {
        m_pipelineOccusionPass->SetShader(ade::Shader::Shader::VertexShader, shaderOccusionPass);

        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{ 0, 0, 3, VT_FLOAT32, False },

            LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 2, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 3, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 4, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
        };

        m_pipelineOccusionPass->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    shaderOccusionPass->GetShaderParameterByName("CBMatrix").Set(u_viewProj);
    //////////////////////////////////////////////////////////////////////////
    uint32_t numInstances = 0;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;

    std::shared_ptr<Buffer> instanceBuffer;

    const uint16_t instanceStride = sizeof(InstanceData);

    // render all instances of the occluder meshes
    for (uint16_t i = 0; i < m_pSceneGdr->m_noofProps; i++) {
        Prop& prop = m_pSceneGdr->m_props[i];

        if (prop.m_renderPass & RenderPass::Occlusion) {
            numInstances = prop.m_noofInstances;

            // render instances to the occlusion buffer
            {
                InstanceData* pData = new InstanceData[numInstances];
                InstanceData* data = pData;

                for (uint32_t j = 0; j < numInstances; j++) {
                    // we only need the world matrix for the occlusion pass
                    // Diligent::float4x4 worldMat = prop.m_instances[j].m_world.Transpose();
                    // ade::memCopy(&data->m_world, &worldMat, sizeof(data->m_world));
                    ade::memCopy(&data->m_world, &prop.m_instances[j].m_world,
                                 sizeof(data->m_world));
                    data++;
                }

                instanceBuffer = Scene::CreateFloatVertexBuffer(App::s_device, (float*)pData,
                                                                numInstances, instanceStride);

                delete pData;
            }

            vertexBuffer = prop.m_vertexbufferHandle;
            indexBuffer = prop.m_indexbufferHandle;

            break;
        }
    }

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Setup the occlusion pass projection
        ade::mtxProj((float*)m_occlusionProj.m, 60.0f, float(m_hiZwidth) / float(m_hiZheight), 0.1f,
                     500.0f, false);

        m_mainView = App::s_eventArgs.pCamera->GetViewMatrix();

        Diligent::float4x4 viewProj = m_mainView * m_occlusionProj;
        u_viewProj->Set(viewProj);

        m_pipelineOccusionPass->Bind();
        renderTarget->Clear();

        // bgfx::setViewFrameBuffer(RENDER_PASS_HIZ_ID, m_hiZDepthBuffer);
        // bgfx::setViewRect(RENDER_PASS_HIZ_ID, 0, 0, uint16_t(m_hiZwidth), uint16_t(m_hiZheight));
        // App::s_backBuffer->SetViewports();
        // Set vertex and index buffer.
        SetVertexBuffer(0, vertexBuffer);
        // SetIndexBuffer(prop.m_indexbufferHandle);

        // Set instance data buffer.
        SetInstanceBuffer(1, instanceBuffer);

        Submit(indexBuffer, numInstances);
    }));
}

// downscale the occluder depth buffer to create a mipmap chain
void TechniqueGdr::renderDownscalePass()
{
    uint32_t width = m_hiZwidth;
    uint32_t height = m_hiZheight;

    // copy mip zero over to the hi Z buffer.
    // We can't currently use blit as it requires same format and CopyResource is not exposed.
    {
        Data4Floats_t inputRendertargetSize{ (float)width, (float)height, 0.0f, 0.0f };
        std::shared_ptr<ade::ConstantBuffer> inputRTSize =
            std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
        inputRTSize->Set(inputRendertargetSize);
        u_inputRTSize.push_back(inputRTSize);

        Diligent::uint3 numThreadGroups = Diligent::uint3(width / 16, height / 16, 1);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        std::shared_ptr<ade::Shader> programCopyZ =
            loadProgram("cs_gdr_copy_z.sh", ade::Shader::ComputeShader);
        dispatchPipeline->SetShader(Shader::ComputeShader, programCopyZ);

        programCopyZ->GetShaderParameterByName("InputRTSize").Set(inputRTSize);

        auto hiZBuffer = m_hiZBuffers[0];

        programCopyZ->GetShaderParameterByName("s_texOcclusionDepth").Set(m_hiZDepthBuffer);
        programCopyZ->GetShaderParameterByName("u_texOcclusionDepthOut").Set(hiZBuffer);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }

    {
        for (uint8_t lod = 1; lod < m_noofHiZMips; ++lod) {
            Data4Floats_t inputRendertargetSize{ (float)width, (float)height, 2.0f, 2.0f };

            std::shared_ptr<ade::ConstantBuffer> inputRTSize =
                std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
            inputRTSize->Set(inputRendertargetSize);
            u_inputRTSize.push_back(inputRTSize);

            inputRTSize->Set(inputRendertargetSize);

            // down scale mip 1 onwards
            width /= 2;
            height /= 2;

            Diligent::uint3 numThreadGroups = Diligent::uint3(
                ade::max((uint32_t)(width / 16), 1u), ade::max((uint32_t)(height / 16), 1u), 1u);

            std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
                std::make_shared<ade::PipelineDispatch>(numThreadGroups);

            std::shared_ptr<Shader> programDownscaleHiZ =
                loadProgram("cs_gdr_downscale_hi_z.sh", ade::Shader::ComputeShader);

            dispatchPipeline->SetShader(Shader::ComputeShader, programDownscaleHiZ);

            programDownscaleHiZ->GetShaderParameterByName("InputRTSize").Set(inputRTSize);

            std::shared_ptr<Texture> texLasMip = m_hiZBuffers[lod - 1];
            std::shared_ptr<Texture> texMip = m_hiZBuffers[lod];

            //// hold it
            // m_hizTexMips.push_back(texLasMip);
            // m_hizTexMips.push_back(texMip);

            programDownscaleHiZ->GetShaderParameterByName("s_texOcclusionDepth").Set(texLasMip);
            programDownscaleHiZ->GetShaderParameterByName("u_texOcclusionDepthOut").Set(texMip);

            std::shared_ptr<PassDispatch> dispatchPass =
                std::make_shared<PassDispatch>(this, dispatchPipeline);

            AddPass(dispatchPass);
        }
    };
}

// perform the occlusion using the mip chain
void TechniqueGdr::renderOccludePropsPass()
{
    // store a rounded-up, power of two instance count for the stream compaction step
    float noofInstancesPowOf2 = ade::pow(
        2.0f, ade::floor(ade::log(m_pSceneGdr->m_totalInstancesCount) / ade::log(2.0f)) + 1.0f);

    Data4Floats_t cullingConfig = { (float)m_pSceneGdr->m_totalInstancesCount, noofInstancesPowOf2,
                                    (float)m_noofHiZMips, (float)m_pSceneGdr->m_noofProps };

    {
        uint16_t groupX = ade::max<uint16_t>(m_pSceneGdr->m_totalInstancesCount / 64 + 1, 1);
        Diligent::uint3 numThreadGroups;
        numThreadGroups.x = groupX;
        numThreadGroups.y = 1;
        numThreadGroups.z = 1;

        // Create programs from shaders for occlusion pass.
        m_programOccludeProps = loadProgram("cs_gdr_occlude_props.sh", ade::Shader::ComputeShader);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programOccludeProps);

        m_programOccludeProps->GetShaderParameterByName("CBMatrix").Set(u_viewProj);
        // run the computer shader to determine visibility of each instance
        // bgfx::setTexture(0, s_texOcclusionDepth, bgfx::getTexture(m_hiZBuffer, 0));

        // bgfx::setBuffer(1, m_instanceBoundingBoxes, bgfx::Access::Read);
        // bgfx::setBuffer(2, m_drawcallInstanceCounts, bgfx::Access::ReadWrite);
        // bgfx::setBuffer(3, m_instancePredicates, bgfx::Access::Write);
        auto hiZBuffer = m_hiZBuffers[0];
        m_programOccludeProps->GetShaderParameterByName("s_texOcclusionDepth").Set(hiZBuffer);
        m_programOccludeProps->GetShaderParameterByName("instanceDataIn")
            .Set(m_instanceBoundingBoxes);
        m_programOccludeProps->GetShaderParameterByName("drawcallInstanceCount")
            .Set(m_drawcallInstanceCounts);
        m_programOccludeProps->GetShaderParameterByName("instancePredicates")
            .Set(m_instancePredicates);
        m_programOccludeProps->GetShaderParameterByName("CullingConfig").Set(u_cullingConfig);

        Data4Floats_t inputRendertargetSize{ (float)m_hiZwidth, (float)m_hiZheight,
                                             1.0f / m_hiZwidth, 1.0f / m_hiZheight };

        std::shared_ptr<ade::ConstantBuffer> inputRTSize =
            std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));

        u_inputRTSize.push_back(inputRTSize);

        m_programOccludeProps->GetShaderParameterByName("InputRTSize").Set(inputRTSize);

        inputRTSize->Set(inputRendertargetSize);

        u_cullingConfig->Set(cullingConfig);

        // set the view/projection transforms so that the compute shader can receive the
        // viewProjection matrix automagically
        // bgfx::setViewTransform(RENDER_PASS_OCCLUDE_PROPS_ID, m_mainView, m_occlusionProj);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }

    {
        // perform stream compaction to remove occluded instances
        m_programStreamCompaction =
            loadProgram("cs_gdr_stream_compaction.sh", ade::Shader::ComputeShader);

        Diligent::uint3 numThreadGroups = Diligent::uint3(1, 1, 1);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programStreamCompaction);

        // the per drawcall data that is constant (noof indices/vertices and offsets to vertex/index
        // buffers)
        m_programStreamCompaction->GetShaderParameterByName("drawcallConstData")
            .Set(m_indirectBufferData);
        //// instance data for all instances (pre culling)
        m_programStreamCompaction->GetShaderParameterByName("instanceDataIn").Set(m_instanceBuffer);
        //// per instance visibility (output of culling pass)
        m_programStreamCompaction->GetShaderParameterByName("instancePredicates")
            .Set(m_instancePredicates);
        //// how many instances per drawcall
        m_programStreamCompaction->GetShaderParameterByName("drawcallInstanceCount")
            .Set(m_drawcallInstanceCounts);
        //// drawcall data that will drive drawIndirect
        m_programStreamCompaction->GetShaderParameterByName("drawcallData").Set(m_indirectBuffer);
        //// culled instance data
        m_programStreamCompaction->GetShaderParameterByName("instanceDataOut")
            .Set(m_culledInstanceBuffer);
        m_programStreamCompaction->GetShaderParameterByName("CullingConfig").Set(u_cullingConfig);

        u_cullingConfig->Set(cullingConfig);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }
}

// render the unoccluded props to the screen
void TechniqueGdr::renderMainPass()
{
    // std::shared_ptr<ade::RenderTarget> renderTarget = std::make_shared<ade::RenderTarget>();
    // auto color0 = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
    // renderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0, color0);
    // renderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_hiZDepthBuffer);
    std::shared_ptr<ade::RenderTarget> renderTarget = m_pRenderTarget;

    std::shared_ptr<Shader> vs = std::make_shared<ade::Shader>();
    vs->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                           "vs_gdr_instanced_indirect_rendering.sh", "main", "./gdr", false);

    std::shared_ptr<Shader> ps = std::make_shared<ade::Shader>();
    ps->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                           "fs_gdr_instanced_indirect_rendering.sh", "main", "./gdr", false);

    vs->GetShaderParameterByName("CBMatrix").Set(u_viewProj);
    ps->GetShaderParameterByName("MaterialColors").Set(u_color);


    m_pipelineMainPass = std::make_shared<Pipeline>(renderTarget);

    m_pipelineMainPass->SetShader(Shader::VertexShader, vs);
    m_pipelineMainPass->SetShader(Shader::PixelShader, ps);


    LayoutElement LayoutElems[] = {
        // Attribute 0 - vertex position
        LayoutElement{ 0, 0, 3, VT_FLOAT32, False },

        LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset, sizeof(InstanceData),
                       LayoutElement::FREQUENCY_PER_INSTANCE },
        LayoutElement{ 2, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset, sizeof(InstanceData),
                       LayoutElement::FREQUENCY_PER_INSTANCE },
        LayoutElement{ 3, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset, sizeof(InstanceData),
                       LayoutElement::FREQUENCY_PER_INSTANCE },
        LayoutElement{ 4, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset, sizeof(InstanceData),
                       LayoutElement::FREQUENCY_PER_INSTANCE },
    };

    m_pipelineMainPass->SetInputLayout(LayoutElems, _countof(LayoutElems));

    uint32_t numInstances = 0;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;

    std::shared_ptr<Buffer> instanceBuffer;

    for (uint16_t ii = 0; ii < m_pSceneGdr->m_noofProps; ++ii) {
        Prop& prop = m_pSceneGdr->m_props[ii];

        if (prop.m_renderPass & RenderPass::MainPass) {
            numInstances = prop.m_noofInstances;

            {
                const uint16_t instanceStride = sizeof(InstanceData);

                InstanceData* pData = new InstanceData[numInstances];
                InstanceData* data = pData;

                for (uint32_t jj = 0; jj < numInstances; ++jj) {
                    // copy world matrix
                    ade::memCopy(&data->m_world, &prop.m_instances[jj].m_world,
                                 sizeof(data->m_world));
                    // pack the material ID into the world transform
                    data->m_world._14 = float(prop.m_materialID);
                    data++;
                }

                instanceBuffer = Scene::CreateFloatVertexBuffer(App::s_device, (float*)data,
                                                                numInstances, instanceStride);
                delete pData;

                // Set vertex and index buffer.
                vertexBuffer = prop.m_vertexbufferHandle;
                indexBuffer = prop.m_indexbufferHandle;

                break;
            }
        }
    }

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        const uint16_t instanceStride = sizeof(InstanceData);

        // Set "material" data (currently a color only)
        u_color->Set(m_pSceneGdr->m_materials,
                     sizeof(m_pSceneGdr->m_materials[0]) * m_pSceneGdr->m_noofMaterials);

        Diligent::float4x4 projMat = App::s_eventArgs.pCamera->GetProjectionMatrix();
        m_mainView = App::s_eventArgs.pCamera->GetViewMatrix();

        Diligent::float4x4 viewProj = m_mainView * projMat;
        u_viewProj->Set(viewProj);

        m_pipelineMainPass->Bind();

        // We can't use indirect drawing for the first frame because the content of
        // m_drawcallInstanceCounts is initially undefined.
        if (m_useIndirect && !m_firstFrame) {
            // Set vertex and index buffer.
            SetVertexBuffer(0, m_allPropsVertexbufferHandle);
            SetVertexBuffer(1, m_indirectBuffer);

            // Set instance data buffer.
            SetInstanceBuffer(1, m_culledInstanceBuffer);

            Submit(m_allPropsIndexbufferHandle, 1);
        } else {
            // render all props using regular instancing
            // Set vertex and index buffer.
            SetVertexBuffer(0, vertexBuffer);
            // Set instance data buffer.
            SetInstanceBuffer(1, instanceBuffer);

            Submit(indexBuffer, numInstances);
        }

        m_firstFrame = false;
    }));
}

void TechniqueGdr::initGdr()
{
    {
        u_viewProj = std::make_shared<ConstantBuffer>((uint32_t)sizeof(CBMatrix));
        u_color = std::make_shared<ConstantBuffer>((uint32_t)sizeof(CBColors));

        createHiZBuffers();

        std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
        AddPass(pSetRTPass);

        std::shared_ptr<PassClearRT> pClearRTPass =
            std::make_shared<PassClearRT>(this, m_pRenderTarget);
        AddPass(pClearRTPass);

        renderOcclusionBufferPass();

        renderDownscalePass();

        renderOccludePropsPass();

        renderMainPass();

        //
        {
            auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
            auto dstTexture = m_pBackBuffer;

            std::shared_ptr<PassCopyTexture> pCopyTexPass =
                std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
            AddPass(pCopyTexPass);
        }
    }
}

void TechniqueGdr::init()
{
    ClearPasses();

    if (m_bDebug) {
        initDebug();
    } else {
        initGdr();
    }
}
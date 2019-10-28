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

const bool bUseNew = false;

TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
    if (!bUseNew) {
        std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
        AddPass(pSetRTPass);

        std::shared_ptr<PassClearRT> pClearRTPass =
            std::make_shared<PassClearRT>(this, m_pRenderTarget);
        AddPass(pClearRTPass);

        bool bUse = true;

        if (bUse) {
            m_pSceneGdr = std::make_shared<SceneGdr>();
            m_pSceneGdr->create();
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

    bool bTemp = false;
    ImGui::Checkbox("debug", &bTemp);
    ImGui::Separator();
}

void TechniqueGdr::createHiZBuffers()
{
    {
        // Create depth buffer
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZDepthBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = m_hiZwidth;
        DepthBufferDesc.Height = m_hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_R32_TYPELESS;    // TEX_FORMAT_R24G8_TYPELESS;
        DepthBufferDesc.SampleCount = 1;                     // App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_RENDER_TARGET;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        m_hiZDepthBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);
    }
    {
        // Create depth buffer
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = m_hiZwidth;
        DepthBufferDesc.Height = m_hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_R32_TYPELESS;
        DepthBufferDesc.SampleCount = 1;    // App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_RENDER_TARGET;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        m_hiZBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);
    }

    // how many mip will the Hi Z buffer have?
    m_noofHiZMips = (uint8_t)(1 + ade::floor(ade::log2(float(ade::max(m_hiZwidth, m_hiZheight)))));

    // Create uniforms and samplers.
    u_inputRTSize = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
    u_cullingConfig = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));

    SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                     FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                     TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };

    StaticSamplerDesc samplerDesc{ SHADER_TYPE_PIXEL, "s_texOcclusionDepth", linearRepeatSampler };

    s_texOcclusionDepth = std::make_shared<SamplerState>(samplerDesc);

    {
        // The compute shader will write how many unoccluded instances per drawcall there are here
        m_drawcallInstanceCounts =
            ade::Scene::CreateUIntIndexBuffer(ade::App::s_device, nullptr, s_maxNoofProps);

        // the compute shader will write the result of the occlusion test for each instance here
        m_instancePredicates =
            ade::Scene::CreateUIntIndexBuffer(ade::App::s_device, nullptr, s_maxNoofInstances);
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

        m_instanceBoundingBoxes = Scene::CreateFloatVertexBuffer(App::s_device, boundingBoxes,
                                                                 m_pSceneGdr->m_totalInstancesCount,
                                                                 sizeof(Diligent::float4) * 2);
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
        m_instanceBuffer = Scene::CreateFloatVertexBuffer(App::s_device, instanceData,
                                                          m_pSceneGdr->m_totalInstancesCount,
                                                          sizeof(Diligent::float4));
        // post occlusion buffer
        m_culledInstanceBuffer = Scene::CreateFloatVertexBuffer(
            App::s_device, nullptr, m_pSceneGdr->m_totalInstancesCount, sizeof(uint32_t));
    }

    // we use one "drawcall" per prop to render all its instances
    const uint kCONFIG_DRAW_INDIRECT_STRIDE = 32;

    m_indirectBuffer = Scene::CreateFloatVertexBuffer(
        App::s_device, nullptr, m_pSceneGdr->m_noofProps, kCONFIG_DRAW_INDIRECT_STRIDE);


    // Create programs from shaders for occlusion pass.
    m_programOcclusionPass = loadProgram("vs_gdr_render_occlusion.sh", ade::Shader::VertexShader);
    m_programCopyZ = loadProgram("cs_gdr_copy_z.sh", ade::Shader::ComputeShader);

    m_programDownscaleHiZ = loadProgram("cs_gdr_downscale_hi_z.sh", ade::Shader::ComputeShader);
    m_programOccludeProps = loadProgram("cs_gdr_occlude_props.sh", ade::Shader::ComputeShader);
    m_programStreamCompaction =
        loadProgram("cs_gdr_stream_compaction.sh", ade::Shader::ComputeShader);

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
    m_indirectBufferData =
        Scene::CreateFloatVertexBuffer(App::s_device, (float*)m_indirectBufferDataCPU,
                                       m_pSceneGdr->m_noofProps, 3 * sizeof(uint32_t));

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
    m_pipelineOccusionPass = std::make_shared<Pipeline>(m_pRenderTarget);

    {
        m_pipelineOccusionPass->SetShader(ade::Shader::Shader::VertexShader,
                                          m_programOcclusionPass);

        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
            // LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
            //               sizeof(InstanceData), LayoutElement::FREQUENCY_PER_INSTANCE },
        };

        m_pipelineOccusionPass->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    std::shared_ptr<ade::RenderTarget> renderTarget = std::make_shared<ade::RenderTarget>();
    renderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_hiZDepthBuffer);

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Setup the occlusion pass projection
        ade::mtxProj((float*)m_occlusionProj.m, 60.0f, float(m_hiZwidth) / float(m_hiZheight), 0.1f,
                     500.0f, false);

        m_mainView = App::s_eventArgs.pCamera->GetViewMatrix();

        m_pipelineOccusionPass->Bind();
        renderTarget->Bind();

        // bgfx::setViewFrameBuffer(RENDER_PASS_HIZ_ID, m_hiZDepthBuffer);
        // bgfx::setViewRect(RENDER_PASS_HIZ_ID, 0, 0, uint16_t(m_hiZwidth), uint16_t(m_hiZheight));
        // App::s_backBuffer->SetViewports();

        const uint16_t instanceStride = sizeof(InstanceData);

        // render all instances of the occluder meshes
        for (uint16_t i = 0; i < m_pSceneGdr->m_noofProps; i++) {
            Prop& prop = m_pSceneGdr->m_props[i];

            if (prop.m_renderPass & RenderPass::Occlusion) {
                const uint32_t numInstances = prop.m_noofInstances;

                // render instances to the occlusion buffer
                {
                    InstanceData* data = new InstanceData[numInstances];

                    for (uint32_t j = 0; j < numInstances; j++) {
                        // we only need the world matrix for the occlusion pass
                        ade::memCopy(&data->m_world, &prop.m_instances[j].m_world,
                                     sizeof(data->m_world));
                        data++;
                    }

                    std::shared_ptr<Buffer> instanceBuffer = Scene::CreateFloatVertexBuffer(
                        App::s_device, (float*)data, numInstances, instanceStride);


                    // Set vertex and index buffer.
                    SetVertexBuffer(0, prop.m_vertexbufferHandle);
                    // SetIndexBuffer(prop.m_indexbufferHandle);

                    // Set instance data buffer.
                    SetInstanceBuffer(1, instanceBuffer);

                    Submit(prop.m_indexbufferHandle, numInstances);
                }
            }
        }
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

        u_inputRTSize->Set(inputRendertargetSize);

        Diligent::uint3 numThreadGroups = Diligent::uint3(width / 16, height / 16, 1);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programCopyZ);

        m_programCopyZ->GetShaderParameterByName("hiZDepthBuffer").Set(m_hiZDepthBuffer);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }

    {
        for (uint8_t lod = 1; lod < m_noofHiZMips; ++lod) {
            Data4Floats_t inputRendertargetSize{ (float)width, (float)height, 2.0f, 2.0f };
            u_inputRTSize->Set(inputRendertargetSize);

            // down scale mip 1 onwards
            width /= 2;
            height /= 2;

            Diligent::uint3 numThreadGroups = Diligent::uint3(width / 16, height / 16, 1);

            std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
                std::make_shared<ade::PipelineDispatch>(numThreadGroups);

            dispatchPipeline->SetShader(Shader::ComputeShader, m_programCopyZ);

            // bgfx::setImage(0, getTexture(m_hiZBuffer, 0), lod - 1, bgfx::Access::Read);
            // bgfx::setImage(1, getTexture(m_hiZBuffer, 0), lod, bgfx::Access::Write);

            m_programCopyZ->GetShaderParameterByName("hiZDepthBuffer").Set(m_hiZDepthBuffer);

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

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programOccludeProps);

        // run the computer shader to determine visibility of each instance
        // bgfx::setTexture(0, s_texOcclusionDepth, bgfx::getTexture(m_hiZBuffer, 0));

        // bgfx::setBuffer(1, m_instanceBoundingBoxes, bgfx::Access::Read);
        // bgfx::setBuffer(2, m_drawcallInstanceCounts, bgfx::Access::ReadWrite);
        // bgfx::setBuffer(3, m_instancePredicates, bgfx::Access::Write);
        m_programOccludeProps->GetShaderParameterByName("m_hiZBuffer").Set(m_hiZBuffer);
        m_programOccludeProps->GetShaderParameterByName("m_instanceBoundingBoxes")
            .Set(m_instanceBoundingBoxes);
        m_programOccludeProps->GetShaderParameterByName("m_drawcallInstanceCounts")
            .Set(m_drawcallInstanceCounts);
        m_programOccludeProps->GetShaderParameterByName("m_instancePredicates")
            .Set(m_instancePredicates);

        Data4Floats_t inputRendertargetSize{ (float)m_hiZwidth, (float)m_hiZheight,
                                             1.0f / m_hiZwidth, 1.0f / m_hiZheight };
        u_inputRTSize->Set(inputRendertargetSize);

        u_cullingConfig->Set(cullingConfig);

        // set the view/projection transforms so that the compute shader can receive the
        // viewProjection matrix automagically
        // bgfx::setViewTransform(RENDER_PASS_OCCLUDE_PROPS_ID, m_mainView, m_occlusionProj);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }

    {
        Diligent::uint3 numThreadGroups = Diligent::uint3(1, 1, 1);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programStreamCompaction);

        m_programStreamCompaction->GetShaderParameterByName("m_indirectBufferData")
            .Set(m_indirectBufferData);
        m_programStreamCompaction->GetShaderParameterByName("m_instanceBuffer")
            .Set(m_instanceBuffer);
        m_programStreamCompaction->GetShaderParameterByName("m_instancePredicates")
            .Set(m_instancePredicates);

        m_programStreamCompaction->GetShaderParameterByName("m_drawcallInstanceCounts")
            .Set(m_drawcallInstanceCounts);
        m_programStreamCompaction->GetShaderParameterByName("m_indirectBuffer")
            .Set(m_indirectBuffer);
        m_programStreamCompaction->GetShaderParameterByName("m_culledInstanceBuffer")
            .Set(m_culledInstanceBuffer);

        // perform stream compaction to remove occluded instances

        // the per drawcall data that is constant (noof indices/vertices and offsets to vertex/index
        // buffers)
        // bgfx::setBuffer(0, m_indirectBufferData, bgfx::Access::Read);
        //// instance data for all instances (pre culling)
        // bgfx::setBuffer(1, m_instanceBuffer, bgfx::Access::Read);
        //// per instance visibility (output of culling pass)
        // bgfx::setBuffer(2, m_instancePredicates, bgfx::Access::Read);

        //// how many instances per drawcall
        // bgfx::setBuffer(3, m_drawcallInstanceCounts, bgfx::Access::ReadWrite);
        //// drawcall data that will drive drawIndirect
        // bgfx::setBuffer(4, m_indirectBuffer, bgfx::Access::ReadWrite);
        //// culled instance data
        // bgfx::setBuffer(5, m_culledInstanceBuffer, bgfx::Access::Write);

        u_cullingConfig->Set(cullingConfig);

        std::shared_ptr<PassDispatch> dispatchPass =
            std::make_shared<PassDispatch>(this, dispatchPipeline);

        AddPass(dispatchPass);
    }
}

// render the unoccluded props to the screen
void TechniqueGdr::renderMainPass()
{
    std::shared_ptr<Shader> vs = std::make_shared<ade::Shader>();
    vs->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                           "vs_gdr_instanced_indirect_rendering.vsh", "main", "./gdr", false);

    std::shared_ptr<Shader> ps = std::make_shared<ade::Shader>();
    ps->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                           "fs_gdr_instanced_indirect_rendering.psh", "main", "./gdr", false);

    m_pipelineMainPass = std::make_shared<Pipeline>(m_pRenderTarget);

    m_pipelineMainPass->SetShader(Shader::VertexShader, vs);
    m_pipelineMainPass->SetShader(Shader::VertexShader, ps);

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Set view and projection matrix for view 0.
        //{
        //    bgfx::setViewTransform(RENDER_PASS_MAIN_ID, m_mainView, m_mainProj);

        //    // Set view 0 default viewport.
        //    bgfx::setViewRect(RENDER_PASS_MAIN_ID, 0, 0, uint16_t(m_width), uint16_t(m_height));
        //}

        const uint16_t instanceStride = sizeof(InstanceData);

        // Set "material" data (currently a color only)
        u_colors->Set(m_pSceneGdr->m_materials,
                      sizeof(m_pSceneGdr->m_materials[0]) * m_pSceneGdr->m_noofMaterials);

        // We can't use indirect drawing for the first frame because the content of
        // m_drawcallInstanceCounts is initially undefined.
        if (m_useIndirect && !m_firstFrame) {
            m_pipelineMainPass->Bind();

            // Set vertex and index buffer.
            SetVertexBuffer(0, m_allPropsVertexbufferHandle);
            SetVertexBuffer(0, m_indirectBuffer);

            SetIndexBuffer(m_allPropsIndexbufferHandle);

            // Set instance data buffer.
            SetInstanceBuffer(1, m_culledInstanceBuffer);

            Submit(m_allPropsIndexbufferHandle, 1);
        } else {
            // render all props using regular instancing
            for (uint16_t ii = 0; ii < m_pSceneGdr->m_noofProps; ++ii) {
                Prop& prop = m_pSceneGdr->m_props[ii];

                if (prop.m_renderPass & RenderPass::MainPass) {
                    const uint32_t numInstances = prop.m_noofInstances;

                    {
                        InstanceData* data = new InstanceData[numInstances];

                        for (uint32_t jj = 0; jj < numInstances; ++jj) {
                            // copy world matrix
                            ade::memCopy(&data->m_world, &prop.m_instances[jj].m_world,
                                         sizeof(data->m_world));
                            // pack the material ID into the world transform
                            data->m_world._14 = float(prop.m_materialID);
                            data++;
                        }

                        std::shared_ptr<Buffer> instanceBuffer = Scene::CreateFloatVertexBuffer(
                            App::s_device, (float*)data, numInstances, instanceStride);


                        // Set vertex and index buffer.
                        SetVertexBuffer(0, prop.m_vertexbufferHandle);

                        // Set instance data buffer.
                        SetInstanceBuffer(1, instanceBuffer);

                        Submit(prop.m_indexbufferHandle, numInstances);
                    }
                }
            }
        }
        m_firstFrame = false;
    }));
}

void TechniqueGdr::init()
{
    if (bUseNew) {
        m_pSceneGdr = std::make_shared<SceneGdr>();
        m_pSceneGdr->create();

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

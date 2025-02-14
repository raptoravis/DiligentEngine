#include <d3d11.h>
#undef max
#undef min

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

// typedef void (*MultiDrawIndirectFn)(uint32_t _numDrawIndirect, ID3D11Buffer* _ptr, uint32_t
// _offset,
//                                    uint32_t _stride);
// static MultiDrawIndirectFn multiDrawInstancedIndirect;
// static MultiDrawIndirectFn multiDrawIndexedInstancedIndirect;
//
// void nvapiMultiDrawInstancedIndirect(uint32_t _numDrawIndirect, ID3D11Buffer* _ptr,
//                                     uint32_t _offset, uint32_t _stride);
// void nvapiMultiDrawIndexedInstancedIndirect(uint32_t _numDrawIndirect, ID3D11Buffer* _ptr,
//                                            uint32_t _offset, uint32_t _stride);

uint8_t calcNumMips(uint32_t _width, uint32_t _height, uint32_t _depth = 1)
{
    const uint32_t max = ade::max(_width, _height, _depth);
    const uint32_t num = 1 + uint32_t(ade::log2(float(max)));

    return uint8_t(num);
}

ITexture* CreateTextureFromTextures(std::vector<std::shared_ptr<ade::Texture>> textures);

TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer), m_bDebug(false), m_useIndirect(true), m_firstFrame(true),
      m_useMultiDraw(false)
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

    ImGui::Checkbox("indirect", &m_useIndirect);
    if (m_useIndirect) {
        ImGui::Checkbox("use multidraw", &m_useMultiDraw);
    }

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

    // for (uint32_t i = 0; i < m_noofHiZMips; ++i)
    {
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = hiZwidth;
        DepthBufferDesc.Height = hiZheight;
        DepthBufferDesc.MipLevels = m_noofHiZMips;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_R32_FLOAT;
        DepthBufferDesc.SampleCount = 1;    // App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        m_hiZBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);

        // m_hiZBuffers.push_back(hiZBuffer);
        // m_hiZBuffer = Scene::CreateTexture2D((uint16_t)m_hiZwidth, (uint16_t)m_hiZheight, 1,
        //                                     Diligent::TEX_FORMAT_R32_FLOAT, CPUAccess::None,
        //                                     true);

        hiZwidth = ade::max(hiZwidth / 2, 1u);
        hiZheight = ade::max(hiZheight / 2, 1u);
    }

    // Create uniforms and samplers.
    // u_inputRTSize = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
    u_cullingConfig = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));

    {
        // The compute shader will write how many unoccluded instances per drawcall there are here
        m_drawcallInstanceCounts = ade::Scene::CreateDynamicIndexBuffer(
            ade::App::s_device, s_maxNoofProps, Diligent::VALUE_TYPE::VT_UINT32);

        // the compute shader will write the result of the occlusion test for each instance here
        m_instancePredicates = ade::Scene::CreateDynamicIndexBuffer(
            ade::App::s_device, s_maxNoofInstances, Diligent::VALUE_TYPE::VT_UINT32);
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
        m_instanceBoundingBoxes = ade::Scene::CreateVertexBufferFloat(
            ade::App::s_device, boundingBoxes, m_pSceneGdr->m_totalInstancesCount * 2,
            sizeof(Diligent::float4), 4);
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
        m_instanceBuffer = ade::Scene::CreateVertexBufferFloat(
            ade::App::s_device, instanceData, 4 * m_pSceneGdr->m_totalInstancesCount,
            (uint32_t)sizeof(Diligent::float4), 4);

        // post occlusion buffer
        // m_culledInstanceBuffer = std::make_shared<ade::StructuredBuffer>(
        //    nullptr, m_pSceneGdr->m_totalInstancesCount, (uint32_t)sizeof(Diligent::float4),
        //    CPUAccess::None, true);
        m_culledInstanceBuffer = ade::Scene::CreateDynamicVertexBufferFloat(
            ade::App::s_device, 4 * m_pSceneGdr->m_totalInstancesCount,
            (uint32_t)sizeof(Diligent::float4), 4);
    }

    // we use one "drawcall" per prop to render all its instances
    // m_indirectBuffer = std::make_shared<ade::StructuredBuffer>(nullptr, m_pSceneGdr->m_noofProps,
    //                                                           (uint32_t)sizeof(Diligent::uint4),
    //                                                           CPUAccess::None, true);
    m_indirectBuffer =
        ade::Scene::CreateIndirectBuffer(ade::App::s_device, m_pSceneGdr->m_noofProps);

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
        ade::memCopy(propIndicesData, prop.m_indices, prop.m_noofIndices * sizeof(uint32_t));

        propVerticesData += prop.m_noofVertices;
        propIndicesData += prop.m_noofIndices;

        m_indirectBufferDataCPU[i * 3] = prop.m_noofIndices;
        m_indirectBufferDataCPU[i * 3 + 1] = indexBufferOffset;
        m_indirectBufferDataCPU[i * 3 + 2] = vertexBufferOffset;

        indexBufferOffset += prop.m_noofIndices;
        vertexBufferOffset += prop.m_noofVertices;
    }

    // Create master vertex buffer
    m_allPropsVertexbufferHandle = Scene::CreateVertexBufferFloat(
        App::s_device, (float*)m_allPropVerticesDataCPU, totalNoofVertices, sizeof(PosVertex));

    // Create master index buffer.
    m_allPropsIndexbufferHandle =
        Scene::CreateIndexBufferUInt(App::s_device, m_allPropIndicesDataCPU, totalNoofIndices);

    // Create buffer with const drawcall data which will be copied to the indirect buffer later.
    m_indirectBufferData = Scene::CreateIndexBufferUInt(App::s_device, m_indirectBufferDataCPU,
                                                        m_pSceneGdr->m_noofProps * 3, true);

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

void TechniqueGdr::Submit(std::shared_ptr<Buffer> pIndexBuffer, uint32_t instancesCount)
{
    SetIndexBuffer(pIndexBuffer);

    {
        auto count = pIndexBuffer->GetCount();

        DrawIndexedAttribs DrawAttrs;
        DrawAttrs.IndexType = VT_UINT32;    // Index type
        DrawAttrs.NumIndices = count;
        DrawAttrs.NumInstances = instancesCount;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

        App::s_ctx->DrawIndexed(DrawAttrs);
    }
}


void TechniqueGdr::Submit(std::shared_ptr<Buffer> pIndexBuffer, uint32_t instancesCount,
                          std::shared_ptr<Buffer> pIndirectBuffer, uint32_t numOfProps)
{
    const uint32_t CONFIG_DRAW_INDIRECT_STRIDE = 32;

    SetIndexBuffer(pIndexBuffer);

    if (m_useMultiDraw) {
        auto buffer = pIndirectBuffer->GetBuffer();

        App::s_ctx->MultiDrawIndexedInstancedIndirect(numOfProps, buffer, 0,
                                                      CONFIG_DRAW_INDIRECT_STRIDE);
    } else {
        auto pIndBuffer = pIndirectBuffer->GetBuffer();
        uint32_t bufferSize = pIndirectBuffer->GetSize();

        const uint32_t kINDIRECT_ARGS_LENGTH = CONFIG_DRAW_INDIRECT_STRIDE / sizeof(uint32_t);

        //{
        //    std::shared_ptr<Buffer> dstBuffer = std::make_shared<StructuredBuffer>(
        //        nullptr, m_pSceneGdr->m_noofProps * kINDIRECT_ARGS_LENGTH,
        //        (uint32_t)sizeof(uint32_t), CPUAccess::Read, false);

        //    std::shared_ptr<Buffer> dstBufferTemp = std::make_shared<IndexBuffer>(
        //        nullptr, m_pSceneGdr->m_noofProps * kINDIRECT_ARGS_LENGTH,
        //        (uint32_t)sizeof(uint32_t), true, Diligent::VALUE_TYPE::VT_UINT32, CPUAccess::Read,
        //        false);

        //    App::s_ctx->CopyBuffer(pIndBuffer, 0,
        //                           Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        //                           dstBufferTemp->GetBuffer(), 0, bufferSize,
        //                           Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        //    PVoid CpuAddress = nullptr;
        //    App::s_ctx->MapBuffer(dstBuffer->GetBuffer(), MAP_READ, MAP_FLAG_NONE, CpuAddress);
        //    uint32_t* pMultiDrawArgs = (uint32_t*)CpuAddress;
        //    App::s_ctx->UnmapBuffer(dstBuffer->GetBuffer(), MAP_READ);
        //}

        uint32_t numOfUints = bufferSize / sizeof(uint32_t);

        for (uint32_t i = 0; i < numOfUints; i += kINDIRECT_ARGS_LENGTH) {
            DrawIndexedIndirectAttribs DrawAttrs;
            DrawAttrs.IndexType = VT_UINT32;    // Index type
            DrawAttrs.IndirectDrawArgsOffset = i * sizeof(uint32_t);
            // Verify the state of vertex and index buffers
            DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

            App::s_ctx->DrawIndexedIndirect(DrawAttrs, pIndBuffer);
        }
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

                instanceBuffer = Scene::CreateVertexBufferFloat(App::s_device, (float*)pData,
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

        // Set vertex and index buffer.
        SetVertexBuffer(0, vertexBuffer);

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

        std::shared_ptr<Texture> texMip = std::make_shared<ade::Texture>(m_hiZBuffer, 0);

        // hold it
        m_hizTexMips.push_back(texMip);

        programCopyZ->GetShaderParameterByName("t_texOcclusionDepth").Set(m_hiZDepthBuffer);
        programCopyZ->GetShaderParameterByName("u_texOcclusionDepthOut").Set(texMip);

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
            dispatchPipeline->SetCheckSRVUAV(false);

            programDownscaleHiZ->GetShaderParameterByName("InputRTSize").Set(inputRTSize);

            std::shared_ptr<Texture> texLasMip =
                std::make_shared<ade::Texture>(m_hiZBuffer, lod - 1);
            std::shared_ptr<Texture> texMip = std::make_shared<ade::Texture>(m_hiZBuffer, lod);

            // hold it
            m_hizTexMips.push_back(texLasMip);
            m_hizTexMips.push_back(texMip);

            programDownscaleHiZ->GetShaderParameterByName("t_texOcclusionDepth").Set(texLasMip);
            programDownscaleHiZ->GetShaderParameterByName("u_texOcclusionDepthOut").Set(texMip);

            std::shared_ptr<PassDispatch> dispatchPass =
                std::make_shared<PassDispatch>(this, dispatchPipeline);

            AddPass(dispatchPass);
        }
    };

    // AddPass(std::make_shared<ade::PassInvokeFunction>(this, [=] {
    //    auto hiZBuffer = CreateTextureFromTextures(m_hiZBuffers);

    //    m_hiZBuffer = std::make_shared<ade::Texture>(hiZBuffer);
    //}));
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
        // m_programOccludeProps = loadProgram("cs_gdr_occlude_props.sh",
        // ade::Shader::ComputeShader);
        m_programOccludeProps = std::make_shared<ade::Shader>();
        m_programOccludeProps->LoadShaderFromFile(ade::Shader::ComputeShader,
                                                  "cs_gdr_occlude_props.sh", "main", "./gdr", true);

        std::shared_ptr<ade::PipelineDispatch> dispatchPipeline =
            std::make_shared<ade::PipelineDispatch>(numThreadGroups);

        dispatchPipeline->SetShader(Shader::ComputeShader, m_programOccludeProps);

        m_programOccludeProps->GetShaderParameterByName("CBMatrix").Set(u_viewProj);

        // run the computer shader to determine visibility of each instance
        auto hiZBuffer = m_hiZBuffer;
        m_programOccludeProps->GetShaderParameterByName("t_texOcclusionDepth").Set(hiZBuffer);

        {
            SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                             FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                             TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };

            StaticSamplerDesc linearRepeatSamplerDesc{ SHADER_TYPE_COMPUTE, "t_texOcclusionDepth",
                                                       linearRepeatSampler };
            t_texOcclusionDepth_sampler =
                std::make_shared<ade::SamplerState>(linearRepeatSamplerDesc);
            m_programOccludeProps->GetShaderParameterByName("t_texOcclusionDepth_sampler")
                .Set(t_texOcclusionDepth_sampler);
        }

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

std::shared_ptr<ade::Pipeline>
    TechniqueGdr::createMainPassPipeline(std::shared_ptr<ade::RenderTarget> renderTarget)
{
    std::shared_ptr<Shader> vs = std::make_shared<ade::Shader>();
    vs->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                           "vs_gdr_instanced_indirect_rendering.sh", "main", "./gdr", false);

    std::shared_ptr<Shader> ps = std::make_shared<ade::Shader>();
    ps->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                           "fs_gdr_instanced_indirect_rendering.sh", "main", "./gdr", false);

    vs->GetShaderParameterByName("CBMatrix").Set(u_viewProj);
    ps->GetShaderParameterByName("MaterialColors").Set(u_color);


    std::shared_ptr<ade::Pipeline> pipelineMainPass = std::make_shared<Pipeline>(renderTarget);

    pipelineMainPass->SetShader(Shader::VertexShader, vs);
    pipelineMainPass->SetShader(Shader::PixelShader, ps);

    return pipelineMainPass;
}

// render the unoccluded props to the screen
void TechniqueGdr::renderMainPass()
{
    // std::shared_ptr<ade::RenderTarget> renderTarget = std::make_shared<ade::RenderTarget>();
    // auto color0 = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
    // renderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0, color0);
    // renderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_hiZDepthBuffer);
    std::shared_ptr<ade::RenderTarget> renderTarget = m_pRenderTarget;
    m_pipelineMainPassDirect = createMainPassPipeline(renderTarget);

    {
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

        m_pipelineMainPassDirect->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    m_pipelineMainPassIndirect = createMainPassPipeline(renderTarget);
    {
        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{ 0, 0, 3, VT_FLOAT32, False },

            LayoutElement{ 1, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(Diligent::float4x4), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 2, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(Diligent::float4x4), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 3, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(Diligent::float4x4), LayoutElement::FREQUENCY_PER_INSTANCE },
            LayoutElement{ 4, 1, 4, VT_FLOAT32, False, LayoutElement::AutoOffset,
                           sizeof(Diligent::float4x4), LayoutElement::FREQUENCY_PER_INSTANCE },
        };

        m_pipelineMainPassIndirect->SetInputLayout(LayoutElems, _countof(LayoutElems));
    }

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Set "material" data (currently a color only)
        u_color->Set(m_pSceneGdr->m_materials,
                     sizeof(m_pSceneGdr->m_materials[0]) * m_pSceneGdr->m_noofMaterials);

        Diligent::float4x4 projMat = App::s_eventArgs.pCamera->GetProjectionMatrix();
        m_mainView = App::s_eventArgs.pCamera->GetViewMatrix();

        Diligent::float4x4 viewProj = m_mainView * projMat;
        u_viewProj->Set(viewProj);

        // We can't use indirect drawing for the first frame because the content of
        // m_drawcallInstanceCounts is initially undefined.
        if (m_useIndirect /* && !m_firstFrame*/) {
            m_pipelineMainPassIndirect->Bind();

            // Set vertex and instance buffer.
            SetVertexBuffer(0, m_allPropsVertexbufferHandle);

            // Set instance data buffer.
            SetInstanceBuffer(1, m_culledInstanceBuffer);

            Submit(m_allPropsIndexbufferHandle, m_pSceneGdr->m_totalInstancesCount,
                   m_indirectBuffer, m_pSceneGdr->m_noofProps);
        } else {
            m_pipelineMainPassDirect->Bind();

            drawDirect();
        }

        m_firstFrame = false;
    }));
}

void TechniqueGdr::drawDirect()
{
    const uint16_t instanceStride = sizeof(InstanceData);

    // render all props using regular instancing
    for (uint16_t ii = 0; ii < m_pSceneGdr->m_noofProps; ++ii) {
        Prop& prop = m_pSceneGdr->m_props[ii];

        if (prop.m_renderPass & RenderPass::MainPass) {
            uint32_t numInstances = prop.m_noofInstances;

            std::shared_ptr<Buffer> instanceBuffer = prop.m_instancebufferHandle;

            if (!instanceBuffer) {
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

                instanceBuffer = Scene::CreateVertexBufferFloat(App::s_device, (float*)pData,
                                                                numInstances, instanceStride);
                delete pData;

                prop.m_instancebufferHandle = instanceBuffer;
            }

            // Set vertex and index buffer.
            std::shared_ptr<Buffer> vertexBuffer = prop.m_vertexbufferHandle;
            std::shared_ptr<Buffer> indexBuffer = prop.m_indexbufferHandle;

            {
                // Set vertex and index buffer.
                SetVertexBuffer(0, vertexBuffer);

                // Set instance data buffer.
                SetInstanceBuffer(1, instanceBuffer);

                Submit(indexBuffer, numInstances);
            }
        }
    }
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


ITexture* CreateTextureFromTextures(std::vector<std::shared_ptr<ade::Texture>> textures)
{
    auto tex0 = textures[0];
    auto texDesc = tex0->GetTexture()->GetDesc();

    TextureDesc TexDesc;

    TexDesc.Name = "texture";
    TexDesc.Type = RESOURCE_DIM_TEX_2D;
    TexDesc.Width = texDesc.Width;
    TexDesc.Height = texDesc.Height;

    TexDesc.MipLevels = calcNumMips(TexDesc.Width, TexDesc.Height);
    TexDesc.Usage = USAGE_DEFAULT;
    TexDesc.BindFlags = texDesc.BindFlags;
    TexDesc.Format = texDesc.Format;
    TexDesc.CPUAccessFlags = texDesc.CPUAccessFlags;

    std::vector<TextureSubResData> pSubResources(TexDesc.MipLevels);
    std::vector<std::vector<Uint8>> Mips(TexDesc.MipLevels);

    for (Uint32 m = 0; m < TexDesc.MipLevels; ++m) {
        auto texture = textures[m]->GetTexture();

        MappedTextureSubresource TexData;
        ade::App::s_ctx->MapTextureSubresource(texture, 0, 0, MAP_READ, MAP_FLAG_DO_NOT_SYNCHRONIZE,
                                               nullptr, TexData);
        ade::App::s_ctx->UnmapTextureSubresource(texture, 0, 0);

        pSubResources[m].pData = TexData.pData;
        pSubResources[m].Stride = TexData.Stride;
    }

    TextureData TexData;
    TexData.pSubResources = pSubResources.data();
    TexData.NumSubresources = TexDesc.MipLevels;

    ITexture* pTexture = 0;

    ade::App::s_device->CreateTexture(TexDesc, &TexData, &pTexture);

    return pTexture;
}

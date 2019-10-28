#include "techniquegdr.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"

#include "engine/utils/mathutils.h"

#include "../mesh/meshprop.h"

#include "../pass/passgdr.h"
#include "../scene/scenegdr.h"


using namespace ade;

static const uint16_t s_maxNoofProps = 10;

static const uint16_t s_maxNoofInstances = 2048;

TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    {
        m_pSceneGdr = std::make_shared<SceneGdr>();
        m_pSceneGdr->create();
        //////////////////////////////////////////////////////////////////////////
        m_PerObject = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::PerObject));
        m_materialId = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::MaterialId));
        u_colors = std::make_shared<ConstantBuffer>((uint32_t)sizeof(PassGdr::Colors));

        this->Set(PassGdr::kPerObjectName, m_PerObject);
        this->Set(PassGdr::kMaterialIdName, m_materialId);
        this->Set(PassGdr::kColorsName, u_colors);

        // Create uniforms and samplers.
        u_inputRTSize = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));
        u_cullingConfig = std::make_shared<ConstantBuffer>((uint32_t)sizeof(Diligent::float4));

        SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                         FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                         TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };

        StaticSamplerDesc samplerDesc{ SHADER_TYPE_PIXEL, "s_texOcclusionDepth",
                                       linearRepeatSampler };

        s_texOcclusionDepth = std::make_shared<SamplerState>(samplerDesc);

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
    // const float rotSpeed = (Diligent::PI_F / 180.0f) * 100.0f;
    //{
    //    auto rootCube = m_pScene->GetRootNode();
    //    auto local = rootCube->GetLocalTransform();
    //    auto localNew =
    //        Diligent::float4x4::RotationY(rotSpeed * App::s_eventArgs.ElapsedTime) * local;
    //    rootCube->SetLocalTransform(localNew);
    //}

    base::Render();
}

void TechniqueGdr::Update()
{
    ImGui::Separator();

    bool bTemp = false;
    ImGui::Checkbox("debug", &bTemp);
    ImGui::Separator();
}

void TechniqueGdr::init() {}

void TechniqueGdr::createHiZBuffers()
{
    uint32_t hiZwidth = 1024;
    uint32_t hiZheight = 512;


    {
        // Create depth buffer
        Diligent::TextureDesc DepthBufferDesc;
        DepthBufferDesc.Name = "hiZDepthBuffer";
        DepthBufferDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        DepthBufferDesc.Width = hiZwidth;
        DepthBufferDesc.Height = hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_R24G8_TYPELESS;    // TEX_FORMAT_D32_FLOAT;
        DepthBufferDesc.SampleCount = 1;                       // App::s_desc.SamplesCount;
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
        DepthBufferDesc.Width = hiZwidth;
        DepthBufferDesc.Height = hiZheight;
        DepthBufferDesc.MipLevels = 1;
        DepthBufferDesc.ArraySize = 1;
        DepthBufferDesc.Format = TEX_FORMAT_D32_FLOAT;
        DepthBufferDesc.SampleCount = 1;    // App::s_desc.SamplesCount;
        DepthBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        DepthBufferDesc.BindFlags = BIND_RENDER_TARGET;
        DepthBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
        DepthBufferDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthStencilTexture;
        App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
        m_hiZDepthBuffer = std::make_shared<ade::Texture>(pDepthStencilTexture);
    }

    // how many mip will the Hi Z buffer have?
    m_noofHiZMips = (uint8_t)(1 + ade::floor(ade::log2(float(ade::max(hiZwidth, hiZheight)))));

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
    m_programOcclusionPass = loadProgram("vs_gdr_render_occlusion.sh", ade::Shader::VertexShader,
                                         m_pRenderTarget, nullptr);
    m_programCopyZ =
        loadProgram("cs_gdr_copy_z.sh", ade::Shader::ComputeShader, m_pRenderTarget, nullptr);

    m_programDownscaleHiZ = loadProgram("cs_gdr_downscale_hi_z.sh", ade::Shader::ComputeShader,
                                        m_pRenderTarget, nullptr);
    m_programOccludeProps = loadProgram("cs_gdr_occlude_props.sh", ade::Shader::ComputeShader,
                                        m_pRenderTarget, nullptr);
    m_programStreamCompaction = loadProgram("cs_gdr_stream_compaction.sh",
                                            ade::Shader::ComputeShader, m_pRenderTarget, nullptr);

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

std::shared_ptr<ade::Pipeline> TechniqueGdr::loadProgram(const std::string& shader,
                                                         ade::Shader::ShaderType st,
                                                         std::shared_ptr<ade::RenderTarget> rt,
                                                         std::shared_ptr<ade::Scene> scene)
{
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(rt);

    {
        std::shared_ptr<Shader> sh = std::make_shared<ade::Shader>();
        sh->LoadShaderFromFile(st, shader.c_str(), "main", "./gdr", false);

        pipeline->SetShader(st, sh);
    }

    return pipeline;
}

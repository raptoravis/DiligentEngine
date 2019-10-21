#include "../engine.h"

pgPipeline::pgPipeline(std::shared_ptr<pgRenderTarget> rt) : m_pRenderTarget(rt), m_bDirty(true) {}

pgPipeline::~pgPipeline()
{
    // virtual function can not be called in the constructor
    // InitPSODesc();
}

void pgPipeline::InitPSODesc()
{
    m_PSODesc.Name = "pgPipeline PSO";

    // This is a graphics pipeline
    m_PSODesc.IsComputePipeline = false;

    auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    auto color0Format =
        color0 ? color0->GetTexture()->GetDesc().Format : pgApp::s_desc.ColorBufferFormat;

    auto ds = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : pgApp::s_desc.DepthBufferFormat;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = (uint8_t)m_pRenderTarget->GetNumRTVs();

    // Set render target format which is the format of the swap chain's color buffer
    m_PSODesc.GraphicsPipeline.RTVFormats[0] = color0Format;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;

    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::True;

    // Define variable type that will be used by default
    m_PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
}


void pgPipeline::SetShader(Shader::ShaderType type, std::shared_ptr<Shader> pShader)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_Shaders[type] = pShader;

    auto shader = pShader->GetShader();

    if (type == Shader::ShaderType::VertexShader) {
        m_PSODesc.GraphicsPipeline.pVS = shader;
    } else if (type == Shader::ShaderType::PixelShader) {
        m_PSODesc.GraphicsPipeline.pPS = shader;
    } else if (type == Shader::ShaderType::ComputeShader) {
        m_PSODesc.ComputePipeline.pCS = shader;
    } else {
        assert(0);
    }

    m_bDirty = true;
}

std::shared_ptr<Shader> pgPipeline::GetShader(Shader::ShaderType type) const
{
    ShaderMap::const_iterator iter = m_Shaders.find(type);
    if (iter != m_Shaders.end()) {
        return iter->second;
    }

    return nullptr;
}

const pgPipeline::ShaderMap& pgPipeline::GetShaders() const
{
    return m_Shaders;
}

void pgPipeline::SetBlendState(const Diligent::BlendStateDesc& blendState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.BlendDesc = blendState;
    m_bDirty = true;
}

Diligent::BlendStateDesc& pgPipeline::GetBlendState()
{
    return m_PSODesc.GraphicsPipeline.BlendDesc;
}

void pgPipeline::SetRasterizerState(const Diligent::RasterizerStateDesc& rasterizerState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.RasterizerDesc = rasterizerState;
    m_bDirty = true;
}

Diligent::RasterizerStateDesc& pgPipeline::GetRasterizerState()
{
    return m_PSODesc.GraphicsPipeline.RasterizerDesc;
}

void pgPipeline::SetDepthStencilState(const Diligent::DepthStencilStateDesc& depthStencilState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.DepthStencilDesc = depthStencilState;
    m_bDirty = true;
}

Diligent::DepthStencilStateDesc& pgPipeline::GetDepthStencilState()
{
    return m_PSODesc.GraphicsPipeline.DepthStencilDesc;
}

void pgPipeline::SetStencilRef(uint32_t ref)
{
    m_stencilRef = ref;
}

void pgPipeline::SetRenderTarget(std::shared_ptr<pgRenderTarget> renderTarget)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_pRenderTarget = renderTarget;
    m_bDirty = true;
}

std::shared_ptr<pgRenderTarget> pgPipeline::GetRenderTarget() const
{
    return m_pRenderTarget;
}

void pgPipeline::Bind()
{
    if (m_bDirty) {
        auto vars = GetVariableDecalarations();

        m_PSODesc.ResourceLayout.Variables = vars.data();
        m_PSODesc.ResourceLayout.NumVariables = (Diligent::Uint32)vars.size();

        pgApp::s_device->CreatePipelineState(m_PSODesc, &m_pPSO);

        SetStaticVariables();

        // Create a shader resource binding object and bind all static resources in it
        m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

        SetVariables();

        m_bDirty = false;
    }

    if (m_pRenderTarget) {
        m_pRenderTarget->Bind();
    }

    // Set the pipeline state
    pgApp::s_ctx->SetPipelineState(m_pPSO);

    pgApp::s_ctx->SetStencilRef(m_stencilRef);

    for (auto shader : m_Shaders) {
        std::shared_ptr<Shader> pShader = shader.second;
        if (pShader) {
            pShader->Bind();
        }
    }

    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    pgApp::s_ctx->CommitShaderResources(m_pSRB,
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void pgPipeline::UnBind()
{
    if (m_pRenderTarget) {
        m_pRenderTarget->UnBind();
    }

    for (auto shader : m_Shaders) {
        std::shared_ptr<Shader> pShader = shader.second;
        if (pShader) {
            pShader->UnBind();
        }
    }
}

void pgPipeline::SetStaticVariables()
{
    // Static variables never change and are bound directly through the pipeline state object.
    // m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants)
    for (auto shader : m_Shaders) {
        std::shared_ptr<Shader> pShader = shader.second;
        if (pShader) {
            Diligent::SHADER_TYPE st = Diligent::SHADER_TYPE_UNKNOWN;
            if (pShader->GetType() == Shader::ShaderType::VertexShader) {
                st = Diligent::SHADER_TYPE_VERTEX;
            } else if (pShader->GetType() == Shader::ShaderType::PixelShader) {
                st = Diligent::SHADER_TYPE_PIXEL;
            } else if (pShader->GetType() == Shader::ShaderType::ComputeShader) {
                st = Diligent::SHADER_TYPE_COMPUTE;
            } else {
                CHECK_ERR(false, "SetStaticVariables");
            }

            auto cbs = pShader->GetConstantBuffers();

            if (cbs.size() > 0) {
                for (auto p : cbs) {
                    if (std::shared_ptr<pgObject> pResource = p->Get().lock()) {
                        if (p->GetType() == ShaderParameter::Type::CBuffer) {
                            std::shared_ptr<ConstantBuffer> cb =
                                std::dynamic_pointer_cast<ConstantBuffer>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(cb->GetBuffer());
                        } else if (p->GetType() == ShaderParameter::Type::Buffer) {
                            std::shared_ptr<StructuredBuffer> res =
                                std::dynamic_pointer_cast<StructuredBuffer>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(res->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWBuffer) {
                            std::shared_ptr<StructuredBuffer> res =
                                std::dynamic_pointer_cast<StructuredBuffer>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(res->GetUnorderedAccessView());
                        } else if (p->GetType() == ShaderParameter::Type::Texture) {
                            std::shared_ptr<pgTexture> res =
                                std::dynamic_pointer_cast<pgTexture>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(res->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWTexture) {
                            std::shared_ptr<pgTexture> res =
                                std::dynamic_pointer_cast<pgTexture>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(res->GetUnorderedAccessView());
                        } else {
                            CHECK_ERR(false, "unsupported variable in SetStaticVariables");
                        }
                    }
                }
            }
        }
    }
}

std::vector<Diligent::ShaderResourceVariableDesc> pgPipeline::GetVariableDecalarations() const
{
    std::vector<Diligent::ShaderResourceVariableDesc> vars;

    for (auto shader : m_Shaders) {
        std::shared_ptr<Shader> pShader = shader.second;
        if (pShader) {
            Diligent::SHADER_TYPE st = Diligent::SHADER_TYPE_UNKNOWN;
            if (pShader->GetType() == Shader::ShaderType::VertexShader) {
                st = Diligent::SHADER_TYPE_VERTEX;
            } else if (pShader->GetType() == Shader::ShaderType::PixelShader) {
                st = Diligent::SHADER_TYPE_PIXEL;
            } else if (pShader->GetType() == Shader::ShaderType::ComputeShader) {
                st = Diligent::SHADER_TYPE_COMPUTE;
            } else {
                CHECK_ERR(0, "unsupported shader type in GetVariableDecalarations");
            }

            auto ncbs = pShader->GetNonConstantBuffers();

            if (ncbs.size() > 0) {
                Diligent::ShaderResourceVariableDesc var;

                var.ShaderStages = st;
                var.Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

                for (auto p : ncbs) {
                    if (p->GetType() != ShaderParameter::Type::Sampler) {
                        var.Name = p->GetName().c_str();

                        vars.push_back(var);
                    }
                }
            }
        }
    }

    return vars;
}

void pgPipeline::SetVariables()
{
    // m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);
    for (auto shader : m_Shaders) {
        std::shared_ptr<Shader> pShader = shader.second;
        if (pShader) {
            Diligent::SHADER_TYPE st = Diligent::SHADER_TYPE_UNKNOWN;
            if (pShader->GetType() == Shader::ShaderType::VertexShader) {
                st = Diligent::SHADER_TYPE_VERTEX;
            } else if (pShader->GetType() == Shader::ShaderType::PixelShader) {
                st = Diligent::SHADER_TYPE_PIXEL;
            } else if (pShader->GetType() == Shader::ShaderType::ComputeShader) {
                st = Diligent::SHADER_TYPE_COMPUTE;
            } else {
                CHECK_ERR(false, "unsupported shader type in SetVariables");
            }

            auto ncbs = pShader->GetNonConstantBuffers();

            if (ncbs.size() > 0) {
                for (auto p : ncbs) {
                    if (std::shared_ptr<pgObject> pResource = p->Get().lock()) {
                        if (p->GetType() == ShaderParameter::Type::RWTexture) {
                            std::shared_ptr<pgTexture> tex =
                                std::dynamic_pointer_cast<pgTexture>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetUnorderedAccessView());
                        } else if (p->GetType() == ShaderParameter::Type::Buffer) {
                            std::shared_ptr<pgBuffer> tex =
                                std::dynamic_pointer_cast<pgBuffer>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWBuffer) {
                            std::shared_ptr<pgBuffer> tex =
                                std::dynamic_pointer_cast<pgBuffer>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetUnorderedAccessView());
                        } else {
                            CHECK_ERR(false, "unsupported variable in SetVariables");
                        }
                    }
                }
            }
        }
    }
}
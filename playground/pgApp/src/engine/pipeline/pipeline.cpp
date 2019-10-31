#include "../engine.h"

using namespace Diligent;

namespace ade
{

Pipeline::Pipeline(std::shared_ptr<RenderTarget> rt)
    : m_pRenderTarget(rt), m_bDirty(true), m_pLayoutElements(nullptr), m_LayoutElements(0)
{
    // virtual function can not be called in the constructor
    // InitPSODesc();
}

Pipeline::~Pipeline()
{
    delete m_pLayoutElements;
}

void Pipeline::InitPSODesc()
{
    m_PSODesc.Name = "Pipeline PSO";

    // This is a graphics pipeline
    m_PSODesc.IsComputePipeline = false;

    auto color0 = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
    auto color0Format =
        color0 ? color0->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    auto ds = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = (uint8_t)m_pRenderTarget->GetNumRTVs();

    // Set render target format which is the format of the swap chain's color buffer
    m_PSODesc.GraphicsPipeline.RTVFormats[0] = color0Format;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;
	
    // Set render target format which is the format of the swap chain's color buffer
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;

    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise =
        RIGHT_HANDED ? RIGHT_HANDED_FRONT_CCW : LEFT_HANDED_FRONT_CCW;

    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::True;

    // Define variable type that will be used by default
    m_PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
}


void Pipeline::SetShader(Shader::ShaderType type, std::shared_ptr<Shader> pShader)
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

std::shared_ptr<Shader> Pipeline::GetShader(Shader::ShaderType type) const
{
    ShaderMap::const_iterator iter = m_Shaders.find(type);
    if (iter != m_Shaders.end()) {
        return iter->second;
    }

    return nullptr;
}

const Pipeline::ShaderMap& Pipeline::GetShaders() const
{
    return m_Shaders;
}

void Pipeline::SetBlendState(const Diligent::BlendStateDesc& blendState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.BlendDesc = blendState;
    m_bDirty = true;
}

Diligent::BlendStateDesc& Pipeline::GetBlendState()
{
    return m_PSODesc.GraphicsPipeline.BlendDesc;
}

void Pipeline::SetRasterizerState(const Diligent::RasterizerStateDesc& rasterizerState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.RasterizerDesc = rasterizerState;
    m_bDirty = true;
}

Diligent::RasterizerStateDesc& Pipeline::GetRasterizerState()
{
    return m_PSODesc.GraphicsPipeline.RasterizerDesc;
}

void Pipeline::SetDepthStencilState(const Diligent::DepthStencilStateDesc& depthStencilState)
{
    if (!m_bInited) {
        m_bInited = true;
        InitPSODesc();
    }

    m_PSODesc.GraphicsPipeline.DepthStencilDesc = depthStencilState;
    m_bDirty = true;
}

Diligent::DepthStencilStateDesc& Pipeline::GetDepthStencilState()
{
    return m_PSODesc.GraphicsPipeline.DepthStencilDesc;
}

void Pipeline::SetInputLayout(Diligent::LayoutElement* pLayoutElements, uint32_t LayoutElements)
{
    delete m_pLayoutElements;

    m_LayoutElements = LayoutElements;
    m_pLayoutElements = new Diligent::LayoutElement[LayoutElements];
    memcpy(m_pLayoutElements, pLayoutElements, sizeof(Diligent::LayoutElement) * LayoutElements);
}


void Pipeline::SetStencilRef(uint32_t ref)
{
    m_stencilRef = ref;
}

// void Pipeline::SetRenderTarget(std::shared_ptr<RenderTarget> renderTarget)
//{
//    if (!m_bInited) {
//        m_bInited = true;
//        InitPSODesc();
//    }
//
//    m_pRenderTarget = renderTarget;
//    m_bDirty = true;
//}

std::shared_ptr<RenderTarget> Pipeline::GetRenderTarget() const
{
    return m_pRenderTarget;
}

//void Pipeline::SetRenderTargetFormat(Diligent::TEXTURE_FORMAT RTFormat,
//                                   Diligent::TEXTURE_FORMAT DSFormat)
//{
//    if (!m_bInited) {
//        m_bInited = true;
//        InitPSODesc();
//    }
//
//    m_PSODesc.GraphicsPipeline.RTVFormats[0] = RTFormat;
//    // Set depth buffer format which is the format of the swap chain's back buffer
//    m_PSODesc.GraphicsPipeline.DSVFormat = DSFormat;
//}

void Pipeline::Bind()
{
    if (m_bDirty) {
        if (m_LayoutElements > 0 && m_pLayoutElements) {
            m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = m_pLayoutElements;
            m_PSODesc.GraphicsPipeline.InputLayout.NumElements = m_LayoutElements;
        }

        auto vars = GetDynamicVariables();

        m_PSODesc.ResourceLayout.Variables = vars.data();
        m_PSODesc.ResourceLayout.NumVariables = (Diligent::Uint32)vars.size();

        auto samplers = GetStaticSamplers();
        m_PSODesc.ResourceLayout.StaticSamplers = samplers.data();
        m_PSODesc.ResourceLayout.NumStaticSamplers = (Diligent::Uint32)samplers.size();

        App::s_device->CreatePipelineState(m_PSODesc, &m_pPSO);

        SetStaticVariables();

        // Create a shader resource binding object and bind all static resources in it
        m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

        SetDynamicVariables();

        m_bDirty = false;
    }

    if (m_pRenderTarget) {
        m_pRenderTarget->Bind();
    }

    // Set the pipeline state
    App::s_ctx->SetPipelineState(m_pPSO);

    App::s_ctx->SetStencilRef(m_stencilRef);

    // for (auto shader : m_Shaders) {
    //    std::shared_ptr<Shader> pShader = shader.second;
    //    if (pShader) {
    //        pShader->Bind();
    //    }
    //}

    Diligent::RESOURCE_STATE_TRANSITION_MODE transitionMode =
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

	bool bCheckUAVSRV = true;
    if (m_Shaders.size() == 1) {
        auto it = m_Shaders.begin();
        auto shader = it->second;
        if (shader->GetType() == Shader::ComputeShader) {
            bCheckUAVSRV = false;
        }
    }

    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    App::s_ctx->CommitShaderResources(m_pSRB, transitionMode, bCheckUAVSRV);
}

void Pipeline::UnBind()
{
    if (m_pRenderTarget) {
        m_pRenderTarget->UnBind();
    }

    // for (auto shader : m_Shaders) {
    //    std::shared_ptr<Shader> pShader = shader.second;
    //    if (pShader) {
    //        pShader->UnBind();
    //    }
    //}
}

void Pipeline::TransitionShaderResources()
{
    App::s_ctx->TransitionShaderResources(m_pPSO, m_pSRB);
}

void Pipeline::SetStaticVariables()
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

            auto cbs = pShader->GetStaticVariables();

            if (cbs.size() > 0) {
                for (auto p : cbs) {
                    if (std::shared_ptr<Object> pResource = p->Get().lock()) {
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
                            std::shared_ptr<Texture> res =
                                std::dynamic_pointer_cast<Texture>(pResource);

                            m_pPSO->GetStaticVariableByName(st, p->GetName().c_str())
                                ->Set(res->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWTexture) {
                            std::shared_ptr<Texture> res =
                                std::dynamic_pointer_cast<Texture>(pResource);

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

std::vector<Diligent::ShaderResourceVariableDesc> Pipeline::GetDynamicVariables() const
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
                CHECK_ERR(0, "unsupported shader type in GetDynamicVariables");
            }

            auto ncbs = pShader->GetDynamicVariables();

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

std::vector<Diligent::StaticSamplerDesc> Pipeline::GetStaticSamplers() const
{
    std::vector<Diligent::StaticSamplerDesc> samplers;

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
                CHECK_ERR(0, "unsupported shader type in GetStaticSamplers");
            }

            auto sams = pShader->GetStaticSamplers();

            if (sams.size() > 0) {
                for (auto p : sams) {
                    if (p->GetType() == ShaderParameter::Type::Sampler) {
                        if (std::shared_ptr<Object> pResource = p->Get().lock()) {
                            std::shared_ptr<SamplerState> s =
                                std::dynamic_pointer_cast<SamplerState>(pResource);

                            auto desc = s->Get();

                            samplers.push_back(desc);
                        }
                    }
                }
            }
        }
    }

    return samplers;
}


void Pipeline::SetDynamicVariables()
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

            auto ncbs = pShader->GetDynamicVariables();

            if (ncbs.size() > 0) {
                for (auto p : ncbs) {
                    if (std::shared_ptr<Object> pResource = p->Get().lock()) {
                        if (p->GetType() == ShaderParameter::Type::Texture) {
                            std::shared_ptr<Texture> tex =
                                std::dynamic_pointer_cast<Texture>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWTexture) {
                            std::shared_ptr<Texture> tex =
                                std::dynamic_pointer_cast<Texture>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetUnorderedAccessView());
                        } else if (p->GetType() == ShaderParameter::Type::Buffer) {
                            std::shared_ptr<Buffer> tex =
                                std::dynamic_pointer_cast<Buffer>(pResource);

                            m_pSRB->GetVariableByName(st, p->GetName().c_str())
                                ->Set(tex->GetShaderResourceView());
                        } else if (p->GetType() == ShaderParameter::Type::RWBuffer) {
                            std::shared_ptr<Buffer> tex =
                                std::dynamic_pointer_cast<Buffer>(pResource);

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

}    // namespace ade
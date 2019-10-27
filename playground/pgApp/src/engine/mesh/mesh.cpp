#include "../engine.h"

using namespace Diligent;
namespace ade
{

Mesh::Mesh()
{
    //
}

Mesh::~Mesh()
{
    //
}

void Mesh::SetInstancesCount(uint32_t instancesCount)
{
    m_instances = instancesCount;
}

void Mesh::AddVertexBuffer(const BufferBinding& binding, std::shared_ptr<Buffer> Buffer)
{
    m_VertexBuffers[binding] = Buffer;
}

void Mesh::SetIndexBuffer(std::shared_ptr<Buffer> Buffer)
{
    m_pIndexBuffer = Buffer;
}

void Mesh::SetMaterial(std::shared_ptr<Material> Material)
{
    m_pMaterial = Material;
}

std::shared_ptr<Material> Mesh::getMaterial() const
{
    return m_pMaterial;
}

static uint32_t getSlot(const BufferBinding& binding)
{
    if (binding.Name == "POSITION") {
        return 0;
    } else if (binding.Name == "VERTEX") {
        return 0;
    } else if (binding.Name == "TANGENT") {
        return 1;
    } else if (binding.Name == "BINORMAL") {
        return 2;
    } else if (binding.Name == "NORMAL") {
        return 3;
    } else if (binding.Name == "TEXCOORD") {
        return 4;
    } else {
        assert(0);
    }

    return 0;
}

void Mesh::Render(Pipeline* pipeline)
{
    std::shared_ptr<Shader> pVS;

    // Clone this mesh's material in case we want to override the
    // shaders in the mesh's default material.
    // Material material( *m_pMaterial );

    if (pipeline) {
        pVS = pipeline->GetShader(Shader::VertexShader);

        if (pVS) {
            for (BufferMap::value_type buffer : m_VertexBuffers) {
                BufferBinding binding = buffer.first;
                // if (pVS->HasSemantic(binding))
                //{
                //	uint32_t slotID = pVS->GetSlotIDBySemantic(binding);
                //	// Bind the vertex buffer to a particular slot ID.
                //	buffer.second->Bind(slotID, Shader::VertexShader,
                // ShaderParameter::Type::Buffer);
                //}
                uint32_t slot = getSlot(binding);

                buffer.second->Bind(slot, Shader::VertexShader, ShaderParameter::Type::Buffer);
            }

            App::s_ctx->SetIndexBuffer(m_pIndexBuffer->GetBuffer(), 0,
                                         RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        if (m_pMaterial) {
            for (auto shader : pipeline->GetShaders()) {
                m_pMaterial->Bind(shader.second);
            }
        }

        auto count = m_pIndexBuffer->GetCount();

        DrawIndexedAttribs DrawAttrs;
        DrawAttrs.IndexType = VT_UINT32;    // Index type
        DrawAttrs.NumIndices = count;
        DrawAttrs.NumInstances = m_instances;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;

        App::s_ctx->DrawIndexed(DrawAttrs);
    } else {
        CHECK_ERR(false, "inavlid pipeline in Mesh::Render");
    }
}

void Mesh::Accept(Visitor& visitor, Pipeline* pipeline)
{
    visitor.Visit(*this, pipeline);
}
}    // namespace ade
#include "../engine.h"

namespace ade
{

SceneNode::SceneNode(const Diligent::float4x4& localTransform)
    : m_LocalTransform(localTransform), m_Name("SceneNode")
{
    m_InverseTransform = m_LocalTransform.Inverse();
}

SceneNode::~SceneNode()
{
    // Delete children.
    m_Children.clear();
}

const std::string& SceneNode::getName() const
{
    return m_Name;
}

void SceneNode::SetName(const std::string& name)
{
    m_Name = name;
}

Diligent::float4x4 SceneNode::GetLocalTransform() const
{
    return m_LocalTransform;
}

void SceneNode::SetLocalTransform(const Diligent::float4x4& localTransform)
{
    m_LocalTransform = localTransform;
    m_InverseTransform = localTransform.Inverse();
}

Diligent::float4x4 SceneNode::GetInverseLocalTransform() const
{
    return m_InverseTransform;
}

Diligent::float4x4 SceneNode::GetWorldTransfom() const
{
    return GetParentWorldTransform() * m_LocalTransform;
}

void SceneNode::SetWorldTransform(const Diligent::float4x4& worldTransform)
{
    Diligent::float4x4 inverseParentTransform = GetParentWorldTransform().Inverse();
    SetLocalTransform(inverseParentTransform * worldTransform);
}

Diligent::float4x4 SceneNode::GetInverseWorldTransform() const
{
    return GetWorldTransfom().Inverse();
}

Diligent::float4x4 SceneNode::GetParentWorldTransform() const
{
    Diligent::float4x4 parentTransform = Diligent::float4x4::Identity();
    if (std::shared_ptr<SceneNode> parent = m_pParentNode.lock()) {
        parentTransform = parent->GetWorldTransfom();
    }

    return parentTransform;
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> pNode)
{
    if (pNode) {
        NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
        if (iter == m_Children.end()) {
            Diligent::float4x4 worldTransform = pNode->GetWorldTransfom();
            pNode->m_pParentNode = shared_from_this();
            Diligent::float4x4 localTransform = GetInverseWorldTransform() * worldTransform;
            pNode->SetLocalTransform(localTransform);
            m_Children.push_back(pNode);
            if (!pNode->getName().empty()) {
                m_ChildrenByName.insert(NodeNameMap::value_type(pNode->getName(), pNode));
            }
        }
    }
}

void SceneNode::RemoveChild(std::shared_ptr<SceneNode> pNode)
{
    if (pNode) {
        NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
        if (iter != m_Children.end()) {
            pNode->SetParent(std::weak_ptr<SceneNode>());

            m_Children.erase(iter);

            // Also remove it from the name map.
            NodeNameMap::iterator iter2 = m_ChildrenByName.find(pNode->getName());
            if (iter2 != m_ChildrenByName.end()) {
                m_ChildrenByName.erase(iter2);
            }
        } else {
            // Maybe this node appears lower in the hierarchy...
            for (auto child : m_Children) {
                child->RemoveChild(pNode);
            }
        }
    }
}

void SceneNode::SetParent(std::weak_ptr<SceneNode> wpNode)
{
    std::shared_ptr<SceneNode> me = shared_from_this();

    if (std::shared_ptr<SceneNode> parent = wpNode.lock()) {
        parent->AddChild(shared_from_this());
    } else if (parent = m_pParentNode.lock()) {
        // Setting parent to NULL.. remove from current parent and reset parent node.
        Diligent::float4x4 worldTransform = GetWorldTransfom();
        parent->RemoveChild(shared_from_this());
        m_pParentNode.reset();
        SetLocalTransform(worldTransform);
    }
}

void SceneNode::AddMesh(std::shared_ptr<Mesh> mesh)
{
    assert(mesh);
    MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
    if (iter == m_Meshes.end()) {
        m_Meshes.push_back(mesh);
    }
}

void SceneNode::RemoveMesh(std::shared_ptr<Mesh> mesh)
{
    assert(mesh);
    MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
    if (iter != m_Meshes.end()) {
        m_Meshes.erase(iter);
    }
}


void SceneNode::Accept(Visitor& visitor, Pipeline* pipeline)
{
    visitor.Visit(*this, pipeline);

    // Visit meshes.
    for (auto mesh : m_Meshes) {
        mesh->Accept(visitor, pipeline);
    }

    // Now visit children
    for (auto child : m_Children) {
        child->Accept(visitor, pipeline);
    }
}

Scene::Scene()
{
    //
}

Scene::~Scene()
{
    //
}

void Scene::Accept(Visitor& visitor, Pipeline* pipeline)
{
    visitor.Visit(*this, pipeline);
    if (m_pRootNode) {
        m_pRootNode->Accept(visitor, pipeline);
    }
}

std::shared_ptr<Texture> Scene::CreateTexture2D(uint16_t width, uint16_t height, uint16_t slices,
                                                Diligent::TEXTURE_FORMAT format,
                                                CPUAccess cpuAccess, bool gpuWrite,
                                                bool bGenerateMipmaps)
{
    Diligent::ITexture* texture = 0;

    Diligent::TextureDesc TexDesc;
    TexDesc.Name = "Scene Texture";
    TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    // TexDesc.Usage = Diligent::USAGE_DEFAULT;
    TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    TexDesc.Width = width;
    TexDesc.Height = height;
    TexDesc.ArraySize = slices;
    TexDesc.Format = format;
    TexDesc.MipLevels = 0;
    TexDesc.MiscFlags = bGenerateMipmaps ? Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS
                                         : Diligent::MISC_TEXTURE_FLAG_NONE;

    // Diligent::RefCntAutoPtr<Diligent::ITexture> texture;

    if ((cpuAccess & CPUAccess::Read) != 0) {
        TexDesc.Usage = Diligent::USAGE_STAGING;
        TexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE | Diligent::CPU_ACCESS_READ;
    } else if ((cpuAccess & (int)CPUAccess::Write) != 0) {
        TexDesc.Usage = Diligent::USAGE_DYNAMIC;
        TexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    } else {
        TexDesc.Usage = Diligent::USAGE_DEFAULT;
        TexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
    }

    //	bool bDynamic = cpuAccess != CPUAccess::None;

    //   if (!gpuWrite && !bDynamic) {
    //       TexDesc.BindFlags |= Diligent::BIND_DEPTH_STENCIL;
    //   }
    //   if (!bDynamic) {
    //       TexDesc.BindFlags |= Diligent::BIND_RENDER_TARGET;
    //   }
    //   if ((cpuAccess & CPUAccess::Read) == 0) {
    //       TexDesc.BindFlags |= Diligent::BIND_SHADER_RESOURCE;
    //   }

    if (gpuWrite) {
        TexDesc.BindFlags |= Diligent::BIND_UNORDERED_ACCESS;
    }

    App::s_device->CreateTexture(TexDesc, nullptr, &texture);

    std::shared_ptr<Texture> tex = std::make_shared<Texture>(texture);

    return tex;
}


std::shared_ptr<Buffer> Scene::CreateVertexBufferFloat(Diligent::IRenderDevice* device, float* data,
                                                       uint32_t count, uint32_t stride,
                                                       uint8_t componentsCount)
{
    bool bSRV = componentsCount > 0;
    bool bDynamic = !data;
    // Create a vertex buffer that stores cube vertices
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Float vertex buffer";

    VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;

    if (bSRV) {
        CHECK_ERR(stride == sizeof(float) * componentsCount, "invalid components count");

        VertBuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
        VertBuffDesc.ElementByteStride = stride;
        VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
        VertBuffDesc.BindFlags |= Diligent::BIND_SHADER_RESOURCE;
    }

    {
        VertBuffDesc.Usage = bDynamic ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_STATIC;
    }

    VertBuffDesc.uiSizeInBytes = stride * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    if (bDynamic) {
        VertBuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

        device->CreateBuffer(VertBuffDesc, nullptr, &pBuffer);
    } else {
        Diligent::BufferData VBData;
        VBData.pData = data;
        VBData.DataSize = stride * count;

        device->CreateBuffer(VertBuffDesc, &VBData, &pBuffer);
    }

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(stride, count, pBuffer);
    if (bSRV) {
        Diligent::VALUE_TYPE ValueType = Diligent::VALUE_TYPE::VT_FLOAT32;
        buffer->SetBufferFormat(ValueType, componentsCount);
    }

    return buffer;
}

std::shared_ptr<Buffer> Scene::CreateIndexBufferUInt(Diligent::IRenderDevice* device,
                                                     uint32_t* data, uint32_t count, bool bSRV)
{
    bool bDynamic = !data;

    Diligent::BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "UInt index buffer";

    IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;

    if (bSRV) {
        IndBuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
        IndBuffDesc.ElementByteStride = sizeof(uint32_t);
        IndBuffDesc.Usage = Diligent::USAGE_DEFAULT;
        IndBuffDesc.BindFlags |= Diligent::BIND_SHADER_RESOURCE;
    }

    {
        IndBuffDesc.Usage = bDynamic ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_STATIC;
    }

    IndBuffDesc.uiSizeInBytes = sizeof(uint32_t) * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    if (bDynamic) {
        IndBuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

        device->CreateBuffer(IndBuffDesc, nullptr, &pBuffer);
    } else {
        Diligent::BufferData IBData;
        IBData.pData = data;
        IBData.DataSize = sizeof(uint32_t) * count;

        device->CreateBuffer(IndBuffDesc, &IBData, &pBuffer);
    }

    std::shared_ptr<Buffer> buffer =
        std::make_shared<Buffer>((uint32_t)sizeof(uint32_t), count, pBuffer);

    if (bSRV) {
        Diligent::VALUE_TYPE ValueType = Diligent::VALUE_TYPE::VT_UINT32;
        buffer->SetBufferFormat(ValueType, 1);
    }

    return buffer;
}

std::shared_ptr<Buffer> Scene::CreateFormatBuffer(Diligent::IRenderDevice* device, void* data,
                                                  Diligent::VALUE_TYPE ValueType, uint32_t count,
                                                  uint32_t stride, bool bSRV, bool bGPUWrite)
{
    bool bDynamic = !data;

    Diligent::BufferDesc BuffDesc;
    BuffDesc.Name = "format buffer";

    if (bSRV) {
        BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
        BuffDesc.ElementByteStride = stride;
        BuffDesc.Usage = Diligent::USAGE_DEFAULT;
        BuffDesc.BindFlags |= Diligent::BIND_SHADER_RESOURCE;
    }

    if (bGPUWrite) {
        BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
        BuffDesc.ElementByteStride = stride;
        BuffDesc.Usage = Diligent::USAGE_DEFAULT;
        BuffDesc.BindFlags |= Diligent::BIND_UNORDERED_ACCESS;
    }

    BuffDesc.uiSizeInBytes = stride * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    if (bDynamic) {
        BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

        device->CreateBuffer(BuffDesc, nullptr, &pBuffer);
    } else {
        Diligent::BufferData IBData;
        IBData.pData = data;
        IBData.DataSize = sizeof(uint32_t) * count;

        device->CreateBuffer(BuffDesc, &IBData, &pBuffer);
    }

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(stride, count, pBuffer);
    buffer->SetBufferFormat(ValueType, 0);

    return buffer;
}

std::shared_ptr<Buffer> Scene::CreateDynamicVertexBuffer(Diligent::IRenderDevice* device,
                                                         uint32_t count, uint32_t stride,
                                                         Diligent::VALUE_TYPE ValueType,
                                                         uint8_t valuesCount)
{
    // Create a vertex buffer that stores cube vertices
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "dynamic vertex buffer";

    VertBuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
    VertBuffDesc.ElementByteStride = stride;

    VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
    VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_SHADER_RESOURCE |
                             Diligent::BIND_UNORDERED_ACCESS;

    VertBuffDesc.uiSizeInBytes = stride * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    device->CreateBuffer(VertBuffDesc, nullptr, &pBuffer);

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(stride, count, pBuffer);
    buffer->SetBufferFormat(ValueType, valuesCount);

    return buffer;
}

std::shared_ptr<Buffer> Scene::CreateDynamicIndexBuffer(Diligent::IRenderDevice* device,
                                                        uint32_t count,
                                                        Diligent::VALUE_TYPE ValueType)
{
    // Create a vertex buffer that stores cube vertices
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "dynamic index buffer";

    CHECK_ERR(ValueType == Diligent::VALUE_TYPE::VT_UINT32 ||
                  ValueType == Diligent::VALUE_TYPE::VT_UINT16,
              "only uint32 or uint16 is allowed");

    uint32_t stride = ValueType == Diligent::VALUE_TYPE::VT_UINT32 ? 4 : 2;
    uint8_t components = 1;

    VertBuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
    VertBuffDesc.ElementByteStride = stride;

    VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
    VertBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER | Diligent::BIND_SHADER_RESOURCE |
                             Diligent::BIND_UNORDERED_ACCESS;

    VertBuffDesc.uiSizeInBytes = stride * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    device->CreateBuffer(VertBuffDesc, nullptr, &pBuffer);

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(stride, count, pBuffer);
    buffer->SetBufferFormat(ValueType, components);

    return buffer;
}

std::shared_ptr<Buffer> Scene::CreateIndirectBuffer(Diligent::IRenderDevice* device, uint32_t count)
{
    // Create a vertex buffer that stores cube vertices
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "indirect buffer";

    uint32_t stride = sizeof(uint32_t) * 4;
    VertBuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
    VertBuffDesc.ElementByteStride = stride;

    VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
    VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_SHADER_RESOURCE |
                             Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_INDIRECT_DRAW_ARGS;

    VertBuffDesc.uiSizeInBytes = stride * count;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> pBuffer;

    device->CreateBuffer(VertBuffDesc, nullptr, &pBuffer);

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(stride, count, pBuffer);
    buffer->SetBufferFormat(Diligent::VALUE_TYPE::VT_UINT32, 4);

    return buffer;
}


}    // namespace ade
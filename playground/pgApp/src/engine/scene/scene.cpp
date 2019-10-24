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

void SceneNode::setName(const std::string& name)
{
    m_Name = name;
}

Diligent::float4x4 SceneNode::getLocalTransform() const
{
    return m_LocalTransform;
}

void SceneNode::setLocalTransform(const Diligent::float4x4& localTransform)
{
    m_LocalTransform = localTransform;
    m_InverseTransform = localTransform.Inverse();
}

Diligent::float4x4 SceneNode::getInverseLocalTransform() const
{
    return m_InverseTransform;
}

Diligent::float4x4 SceneNode::getWorldTransfom() const
{
    return GetParentWorldTransform() * m_LocalTransform;
}

void SceneNode::setWorldTransform(const Diligent::float4x4& worldTransform)
{
    Diligent::float4x4 inverseParentTransform = GetParentWorldTransform().Inverse();
    setLocalTransform(inverseParentTransform * worldTransform);
}

Diligent::float4x4 SceneNode::getInverseWorldTransform() const
{
    return getWorldTransfom().Inverse();
}

Diligent::float4x4 SceneNode::GetParentWorldTransform() const
{
    Diligent::float4x4 parentTransform = Diligent::float4x4::Identity();
    if (std::shared_ptr<SceneNode> parent = m_pParentNode.lock()) {
        parentTransform = parent->getWorldTransfom();
    }

    return parentTransform;
}

void SceneNode::addChild(std::shared_ptr<SceneNode> pNode)
{
    if (pNode) {
        NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
        if (iter == m_Children.end()) {
            Diligent::float4x4 worldTransform = pNode->getWorldTransfom();
            pNode->m_pParentNode = shared_from_this();
            Diligent::float4x4 localTransform = getInverseWorldTransform() * worldTransform;
            pNode->setLocalTransform(localTransform);
            m_Children.push_back(pNode);
            if (!pNode->getName().empty()) {
                m_ChildrenByName.insert(NodeNameMap::value_type(pNode->getName(), pNode));
            }
        }
    }
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> pNode)
{
    if (pNode) {
        NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
        if (iter != m_Children.end()) {
            pNode->setParent(std::weak_ptr<SceneNode>());

            m_Children.erase(iter);

            // Also remove it from the name map.
            NodeNameMap::iterator iter2 = m_ChildrenByName.find(pNode->getName());
            if (iter2 != m_ChildrenByName.end()) {
                m_ChildrenByName.erase(iter2);
            }
        } else {
            // Maybe this node appears lower in the hierarchy...
            for (auto child : m_Children) {
                child->removeChild(pNode);
            }
        }
    }
}

void SceneNode::setParent(std::weak_ptr<SceneNode> wpNode)
{
    std::shared_ptr<SceneNode> me = shared_from_this();

    if (std::shared_ptr<SceneNode> parent = wpNode.lock()) {
        parent->addChild(shared_from_this());
    } else if (parent = m_pParentNode.lock()) {
        // Setting parent to NULL.. remove from current parent and reset parent node.
        Diligent::float4x4 worldTransform = getWorldTransfom();
        parent->removeChild(shared_from_this());
        m_pParentNode.reset();
        setLocalTransform(worldTransform);
    }
}

void SceneNode::addMesh(std::shared_ptr<Mesh> mesh)
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

std::shared_ptr<Texture> Scene::CreateTexture2D(uint16_t width, uint16_t height,
                                                    uint16_t slices,
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

}    // namespace ade
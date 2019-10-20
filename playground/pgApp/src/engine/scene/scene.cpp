#include "../engine.h"

pgSceneNode::pgSceneNode(const Diligent::float4x4& localTransform)
    : m_LocalTransform(localTransform), m_Name("SceneNode")
{
    m_InverseTransform = m_LocalTransform.Inverse();
}

pgSceneNode::~pgSceneNode()
{
    // Delete children.
    m_Children.clear();
}

const std::string& pgSceneNode::getName() const
{
    return m_Name;
}

void pgSceneNode::setName(const std::string& name)
{
    m_Name = name;
}

Diligent::float4x4 pgSceneNode::getLocalTransform() const
{
    return m_LocalTransform;
}

void pgSceneNode::setLocalTransform(const Diligent::float4x4& localTransform)
{
    m_LocalTransform = localTransform;
    m_InverseTransform = localTransform.Inverse();
}

Diligent::float4x4 pgSceneNode::getInverseLocalTransform() const
{
    return m_InverseTransform;
}

Diligent::float4x4 pgSceneNode::getWorldTransfom() const
{
    return GetParentWorldTransform() * m_LocalTransform;
}

void pgSceneNode::setWorldTransform(const Diligent::float4x4& worldTransform)
{
    Diligent::float4x4 inverseParentTransform = GetParentWorldTransform().Inverse();
    setLocalTransform(inverseParentTransform * worldTransform);
}

Diligent::float4x4 pgSceneNode::getInverseWorldTransform() const
{
    return getWorldTransfom().Inverse();
}

Diligent::float4x4 pgSceneNode::GetParentWorldTransform() const
{
    Diligent::float4x4 parentTransform = Diligent::float4x4::Identity();
    if (std::shared_ptr<pgSceneNode> parent = m_pParentNode.lock()) {
        parentTransform = parent->getWorldTransfom();
    }

    return parentTransform;
}

void pgSceneNode::addChild(std::shared_ptr<pgSceneNode> pNode)
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

void pgSceneNode::removeChild(std::shared_ptr<pgSceneNode> pNode)
{
    if (pNode) {
        NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
        if (iter != m_Children.end()) {
            pNode->setParent(std::weak_ptr<pgSceneNode>());

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

void pgSceneNode::setParent(std::weak_ptr<pgSceneNode> wpNode)
{
    std::shared_ptr<pgSceneNode> me = shared_from_this();

    if (std::shared_ptr<pgSceneNode> parent = wpNode.lock()) {
        parent->addChild(shared_from_this());
    } else if (parent = m_pParentNode.lock()) {
        // Setting parent to NULL.. remove from current parent and reset parent node.
        Diligent::float4x4 worldTransform = getWorldTransfom();
        parent->removeChild(shared_from_this());
        m_pParentNode.reset();
        setLocalTransform(worldTransform);
    }
}

void pgSceneNode::addMesh(std::shared_ptr<pgMesh> mesh)
{
    assert(mesh);
    MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
    if (iter == m_Meshes.end()) {
        m_Meshes.push_back(mesh);
    }
}

void pgSceneNode::RemoveMesh(std::shared_ptr<pgMesh> mesh)
{
    assert(mesh);
    MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
    if (iter != m_Meshes.end()) {
        m_Meshes.erase(iter);
    }
}


void pgSceneNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);

    // Visit meshes.
    for (auto mesh : m_Meshes) {
        mesh->Accept(visitor);
    }

    // Now visit children
    for (auto child : m_Children) {
        child->Accept(visitor);
    }
}

pgScene::pgScene()
{
    //
}

pgScene::~pgScene()
{
    //
}

void pgScene::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (m_pRootNode) {
        m_pRootNode->Accept(visitor);
    }
}

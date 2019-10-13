#include "../engine.h"

pgSceneNode::pgSceneNode(const Diligent::float4x4& localTransform)
	: m_LocalTransform(localTransform)
	, m_Name("SceneNode")
{
	m_InverseTransform = m_LocalTransform.Inverse();
}

pgSceneNode::~pgSceneNode()
{
	// Delete children.
	m_Children.clear();
}

const std::string& pgSceneNode::GetName() const
{
	return m_Name;
}

void  pgSceneNode::SetName(const std::string& name)
{
	m_Name = name;
}

Diligent::float4x4 pgSceneNode::GetLocalTransform() const
{
	return m_LocalTransform;
}

void pgSceneNode::SetLocalTransform(const Diligent::float4x4& localTransform)
{
	m_LocalTransform = localTransform;
	m_InverseTransform = localTransform.Inverse();
}

Diligent::float4x4 pgSceneNode::GetInverseLocalTransform() const
{
	return m_InverseTransform;
}

Diligent::float4x4 pgSceneNode::GetWorldTransfom() const
{
	return GetParentWorldTransform() * m_LocalTransform;
}

void pgSceneNode::SetWorldTransform(const Diligent::float4x4& worldTransform)
{
	Diligent::float4x4 inverseParentTransform = GetParentWorldTransform().Inverse();
	SetLocalTransform(inverseParentTransform * worldTransform);
}

Diligent::float4x4 pgSceneNode::GetInverseWorldTransform() const
{
	return GetWorldTransfom().Inverse();
}

Diligent::float4x4 pgSceneNode::GetParentWorldTransform() const
{
	Diligent::float4x4 parentTransform(1.0f);
	if (std::shared_ptr<pgSceneNode> parent = m_pParentNode.lock())
	{
		parentTransform = parent->GetWorldTransfom();
	}

	return parentTransform;
}

void pgSceneNode::AddChild(std::shared_ptr<pgSceneNode> pNode)
{
	if (pNode)
	{
		NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
		if (iter == m_Children.end())
		{
			Diligent::float4x4 worldTransform = pNode->GetWorldTransfom();
			pNode->m_pParentNode = shared_from_this();
			Diligent::float4x4 localTransform = GetInverseWorldTransform() * worldTransform;
			pNode->SetLocalTransform(localTransform);
			m_Children.push_back(pNode);
			if (!pNode->GetName().empty())
			{
				m_ChildrenByName.insert(NodeNameMap::value_type(pNode->GetName(), pNode));
			}
		}
	}
}

void pgSceneNode::RemoveChild(std::shared_ptr<pgSceneNode> pNode)
{
	if (pNode)
	{
		NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
		if (iter != m_Children.end())
		{
			pNode->SetParent(std::weak_ptr<pgSceneNode>());

			m_Children.erase(iter);

			// Also remove it from the name map.
			NodeNameMap::iterator iter2 = m_ChildrenByName.find(pNode->GetName());
			if (iter2 != m_ChildrenByName.end())
			{
				m_ChildrenByName.erase(iter2);
			}
		}
		else
		{
			// Maybe this node appears lower in the hierarchy...
			for (auto child : m_Children)
			{
				child->RemoveChild(pNode);
			}
		}
	}
}

void pgSceneNode::SetParent(std::weak_ptr<pgSceneNode> wpNode)
{
	std::shared_ptr<pgSceneNode> me = shared_from_this();

	if (std::shared_ptr<pgSceneNode> parent = wpNode.lock())
	{
		parent->AddChild(shared_from_this());
	}
	else if (parent = m_pParentNode.lock())
	{
		// Setting parent to NULL.. remove from current parent and reset parent node.
		Diligent::float4x4 worldTransform = GetWorldTransfom();
		parent->RemoveChild(shared_from_this());
		m_pParentNode.reset();
		SetLocalTransform(worldTransform);
	}
}

void pgSceneNode::addMesh(std::shared_ptr<pgMesh> mesh)
{
	assert(mesh);
	MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
	if (iter == m_Meshes.end())
	{
		m_Meshes.push_back(mesh);
	}
}

void pgSceneNode::RemoveMesh(std::shared_ptr<pgMesh> mesh)
{
	assert(mesh);
	MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
	if (iter != m_Meshes.end())
	{
		m_Meshes.erase(iter);
	}
}

void pgSceneNode::Render(pgRenderEventArgs& args)
{
	// First render all my meshes.
	for (auto mesh : m_Meshes)
	{
		mesh->Render(this, args);
	}

	// Now recurse into children
	for (auto child : m_Children)
	{
		child->Render(args);
	}
}

void pgScene::Render(pgRenderEventArgs& args)
{
	if (m_pRootNode)
	{
		m_pRootNode->Render(args);
	}
}

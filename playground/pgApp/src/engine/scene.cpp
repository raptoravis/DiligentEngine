#include "engine.h"

SceneNode::SceneNode(const Diligent::float4x4& localTransform)
	: m_LocalTransform(localTransform)
	, m_Name("SceneNode")
{
	m_InverseTransform = m_LocalTransform.Inverse();
}

SceneNode::~SceneNode()
{
	// Delete children.
	m_Children.clear();
}

const std::string& SceneNode::GetName() const
{
	return m_Name;
}

void  SceneNode::SetName(const std::string& name)
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
	Diligent::float4x4 parentTransform(1.0f);
	if (SceneNode* parent = m_pParentNode)
	{
		parentTransform = parent->GetWorldTransfom();
	}

	return parentTransform;
}

void SceneNode::AddChild(SceneNode* pNode)
{
	if (pNode)
	{
		NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
		if (iter == m_Children.end())
		{
			Diligent::float4x4 worldTransform = pNode->GetWorldTransfom();
			pNode->m_pParentNode = this;
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

void SceneNode::RemoveChild(SceneNode* pNode)
{
	if (pNode)
	{
		NodeList::iterator iter = std::find(m_Children.begin(), m_Children.end(), pNode);
		if (iter != m_Children.end())
		{
			pNode->SetParent(0);

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

void SceneNode::SetParent(SceneNode* wpNode)
{
	if (SceneNode* parent = wpNode)
	{
		parent->AddChild(this);
	}
	else if (SceneNode* parentOld = m_pParentNode)
	{
		// Setting parent to NULL.. remove from current parent and reset parent node.
		Diligent::float4x4 worldTransform = GetWorldTransfom();
		parentOld->RemoveChild(this);
		m_pParentNode = 0;
		SetLocalTransform(worldTransform);
	}
}

void SceneNode::AddMesh(Mesh* mesh)
{
	assert(mesh);
	MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
	if (iter == m_Meshes.end())
	{
		m_Meshes.push_back(mesh);
	}
}

void SceneNode::RemoveMesh(Mesh* mesh)
{
	assert(mesh);
	MeshList::iterator iter = std::find(m_Meshes.begin(), m_Meshes.end(), mesh);
	if (iter != m_Meshes.end())
	{
		m_Meshes.erase(iter);
	}
}

void SceneNode::Render(RenderEventArgs& args)
{
	// First render all my meshes.
	for (auto mesh : m_Meshes)
	{
		mesh->Render(args);
	}

	// Now recurse into children
	for (auto child : m_Children)
	{
		child->Render(args);
	}
}

void Scene::Render(RenderEventArgs& args)
{
	if (m_pRootNode)
	{
		m_pRootNode->Render(args);
	}
}

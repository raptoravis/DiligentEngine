#include "../engine.h"

pgMaterial::pgMaterial(Diligent::IRenderDevice* device)
	: m_RenderDevice(device)
	, m_Dirty(false)
{
	m_pProperties = (MaterialProperties*)_aligned_malloc(sizeof(MaterialProperties), 16);
	// Construct default material properties.
	*m_pProperties = MaterialProperties();

	// Initialize the constant buffer.
	Diligent::BufferDesc VertBuffDesc;
	VertBuffDesc.Name = "Cube vertex buffer";
	VertBuffDesc.Usage = Diligent::USAGE_STATIC;
	VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = sizeof(*m_pProperties);

	Diligent::BufferData VBData;
	VBData.pData = m_pProperties;
	VBData.DataSize = sizeof(*m_pProperties);
	m_RenderDevice->CreateBuffer(VertBuffDesc, &VBData, &m_pConstantBuffer);
}

pgMaterial::~pgMaterial()
{
	if (m_pProperties)
	{
		_aligned_free(m_pProperties);
		m_pProperties = nullptr;
	}
}

void pgMaterial::Bind(Diligent::IShader* pShader) const
{
	if (!pShader) return;

	if (m_Dirty)
	{
		// Make sure the constant buffer associated to this material is updated.
		pgMaterial* _this = const_cast<pgMaterial*>(this);
		_this->UpdateConstantBuffer();
		_this->m_Dirty = false;
	}

	//for (auto texture : m_Textures)
	//{
	//	ITexture* pTexture = texture.second;
	//	pTexture->Bind((uint32_t)texture.first, pShader->GetType(), ShaderParameter::Type::Texture);
	//}

	//// If the shader has a parameter called "Material".
	//ShaderParameter& materialParameter = pShader->GetShaderParameterByName("Material");
	//if (materialParameter.IsValid())
	//{
	//	// Assign this material's constant buffer to it.
	//	materialParameter.Set<ConstantBuffer>(m_pConstantBuffer);
	//	// If the shader parameter is modified, they have to be 
	//	// rebound to update the rendering pipeline.
	//	materialParameter.Bind();
	//}
}

const Diligent::float4& pgMaterial::GetGlobalAmbientColor() const
{
	return m_pProperties->m_GlobalAmbient;
}

void pgMaterial::SetGlobalAmbientColor(const Diligent::float4& globalAmbient)
{
	m_pProperties->m_GlobalAmbient = globalAmbient;
	m_Dirty = true;
}

const Diligent::float4& pgMaterial::GetAmbientColor() const
{
	return m_pProperties->m_AmbientColor;
}

void pgMaterial::SetAmbientColor(const Diligent::float4& ambient)
{
	m_pProperties->m_AmbientColor = ambient;
	m_Dirty = true;
}

const Diligent::float4& pgMaterial::GetDiffuseColor() const
{
	return m_pProperties->m_DiffuseColor;
}

void pgMaterial::SetDiffuseColor(const Diligent::float4& diffuse)
{
	m_pProperties->m_DiffuseColor = diffuse;
	m_Dirty = true;
}

const Diligent::float4& pgMaterial::GetEmissiveColor() const
{
	return m_pProperties->m_EmissiveColor;
}

void pgMaterial::SetEmissiveColor(const Diligent::float4& emissive)
{
	m_pProperties->m_EmissiveColor = emissive;
	m_Dirty = true;
}

const Diligent::float4& pgMaterial::GetSpecularColor() const
{
	return m_pProperties->m_SpecularColor;
}

void pgMaterial::SetSpecularColor(const Diligent::float4& specular)
{
	m_pProperties->m_SpecularColor = specular;
	m_Dirty = true;
}

float pgMaterial::GetSpecularPower() const
{
	return m_pProperties->m_SpecularPower;
}

const float pgMaterial::GetOpacity() const
{
	return m_pProperties->m_Opacity;
}

void pgMaterial::SetOpacity(float Opacity)
{
	m_pProperties->m_Opacity = Opacity;
	m_Dirty = true;
}

void pgMaterial::SetSpecularPower(float phongPower)
{
	m_pProperties->m_SpecularPower = phongPower;
	m_Dirty = true;
}

const Diligent::float4& pgMaterial::GetReflectance() const
{
	return m_pProperties->m_Reflectance;
}

void pgMaterial::SetReflectance(const Diligent::float4& reflectance)
{
	m_pProperties->m_Reflectance = reflectance;
	m_Dirty = true;
}

float pgMaterial::GetIndexOfRefraction() const
{
	return m_pProperties->m_IndexOfRefraction;
}

void pgMaterial::SetIndexOfRefraction(float indexOfRefraction)
{
	m_pProperties->m_IndexOfRefraction = indexOfRefraction;
	m_Dirty = true;
}

float pgMaterial::GetBumpIntensity() const
{
	return m_pProperties->m_BumpIntensity;
}
void pgMaterial::SetBumpIntensity(float bumpIntensity)
{
	m_pProperties->m_BumpIntensity = bumpIntensity;
}

std::shared_ptr<pgTexture> pgMaterial::GetTexture(TextureType type) const
{
	TextureMap::const_iterator itr = m_Textures.find(type);
	if (itr != m_Textures.end())
	{
		return itr->second;
	}

	return nullptr;
}

void pgMaterial::SetTexture(TextureType type, std::shared_ptr<pgTexture> texture)
{
	m_Textures[type] = texture;

	switch (type)
	{
	case TextureType::Ambient:
	{
		m_pProperties->m_HasAmbientTexture = (texture != nullptr);
	}
	break;
	case TextureType::Emissive:
	{
		m_pProperties->m_HasEmissiveTexture = (texture != nullptr);
	}
	break;
	case TextureType::Diffuse:
	{
		m_pProperties->m_HasDiffuseTexture = (texture != nullptr);
	}
	break;
	case TextureType::Specular:
	{
		m_pProperties->m_HasSpecularTexture = (texture != nullptr);
	}
	break;
	case TextureType::SpecularPower:
	{
		m_pProperties->m_HasSpecularPowerTexture = (texture != nullptr);
	}
	break;
	case TextureType::Normal:
	{
		m_pProperties->m_HasNormalTexture = (texture != nullptr);
	}
	break;
	case TextureType::Bump:
	{
		m_pProperties->m_HasBumpTexture = (texture != nullptr);
	}
	break;
	case TextureType::Opacity:
	{
		m_pProperties->m_HasOpacityTexture = (texture != nullptr);
	}
	break;
	}

	m_Dirty = true;
}

bool pgMaterial::IsTransparent() const
{
	return (m_pProperties->m_Opacity < 1.0f ||
		m_pProperties->m_HasOpacityTexture ||
		(m_pProperties->m_HasDiffuseTexture && GetTexture(TextureType::Diffuse)->IsTransparent()) ||
		m_pProperties->m_AlphaThreshold <= 0.0f); // Objects with an alpha threshold > 0 should be drawn in the opaque pass.
}

void pgMaterial::UpdateConstantBuffer()
{
	//if (m_pConstantBuffer)
	//{
	//	m_pConstantBuffer->Set(*m_pProperties);
	//}
}

#include "../engine.h"

uint16_t pgTexture::GetWidth() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Width;
}

uint16_t pgTexture::GetHeight() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Height;
}

uint16_t pgTexture::GetDepth() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Depth;
}

uint8_t pgTexture::GetBPP() const
{
	//auto& desc = m_pTexture->GetDesc();
	//desc.Format;

	return 4;
}

bool pgTexture::IsTransparent() const
{
	//auto& desc = m_pTexture->GetDesc();
	//desc.Format;

	return false;
}

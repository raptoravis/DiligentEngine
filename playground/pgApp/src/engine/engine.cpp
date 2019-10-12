#include "engine.h"

pgBasePass::pgBasePass(const pgPassCreateInfo& ci)
	: base(ci)
{
}

pgBasePass::~pgBasePass()
{
}


void pgBasePass::Update(RenderEventArgs& e)
{
}

void pgBasePass::Render(RenderEventArgs& e)
{
}

void pgBasePass::UpdateUI(RenderEventArgs& e)
{
}


pgTechnique::pgTechnique()
{}

pgTechnique::~pgTechnique()
{}

unsigned int pgTechnique::AddPass(pgPass* pass)
{
	// No check for duplicate passes (it may be intended to render the same pass multiple times?)
	m_Passes.push_back(pass);
	return static_cast<unsigned int>(m_Passes.size()) - 1;
}

pgPass* pgTechnique::GetPass(unsigned int ID) const
{
	if (ID < m_Passes.size())
	{
		return m_Passes[ID];
	}

	return 0;
}

void pgTechnique::Update(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Update(e);
		}
	}
}

// Render the scene using the passes that have been configured.
void pgTechnique::Render(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Render(e);
		}
	}
}



#include "engine.h"

pgTechnique::pgTechnique(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer)
    : m_pRT(rt), m_pBackBuffer(backBuffer)
{
}

pgTechnique::~pgTechnique()
{
    m_Passes.clear();
}

unsigned int pgTechnique::addPass(std::shared_ptr<pgPass> pass)
{
    // No check for duplicate passes (it may be intended to render the same pass multiple times?)
    m_Passes.push_back(pass);
    return static_cast<unsigned int>(m_Passes.size()) - 1;
}

std::shared_ptr<pgPass> pgTechnique::getPass(unsigned int ID) const
{
    if (ID < m_Passes.size()) {
        return m_Passes[ID];
    }

    return 0;
}


void pgTechnique::SetResource(const std::string& name, std::shared_ptr<pgObject> res)
{
    m_resourceMap[name] = res;
}


std::shared_ptr<pgObject> pgTechnique::GetResource(const std::string& name)
{
    ResourceMap::iterator it = m_resourceMap.find(name);

    if (it != m_resourceMap.end()) {
        return it->second;
    }

    return nullptr;
}


void pgTechnique::Render()
{
    for (auto pass : m_Passes) {
        if (pass->IsEnabled()) {
            pass->PreRender();
            pass->Render();
            pass->PostRender();
        }
    }
}
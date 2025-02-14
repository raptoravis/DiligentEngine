#include "../engine.h"

namespace ade
{

Technique::Technique(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : m_pRenderTarget(rt), m_pBackBuffer(backBuffer)
{
}

Technique::~Technique()
{
    m_Passes.clear();
}

uint32_t Technique::AddPass(std::shared_ptr<Pass> pass)
{
    // No check for duplicate passes (it may be intended to render the same pass multiple times?)
    m_Passes.push_back(pass);
    return static_cast<unsigned int>(m_Passes.size()) - 1;
}

std::shared_ptr<Pass> Technique::GetPass(uint32_t ID) const
{
    if (ID < m_Passes.size()) {
        return m_Passes[ID];
    }

    return 0;
}

void Technique::RemovePass(std::shared_ptr<Pass> pass) {
    for (RenderPassList::iterator it = m_Passes.begin(); it != m_Passes.end(); ++it) {
        auto p = *it;
        if (p == pass) {
            m_Passes.erase(it);
            break;
        }
    }
}

void Technique::ClearPasses()
{
    m_Passes.clear();
}


void Technique::Set(const std::string& name, std::shared_ptr<Object> res)
{
    m_resourceMap[name] = res;
}


std::shared_ptr<Object> Technique::Get(const std::string& name)
{
    ResourceMap::iterator it = m_resourceMap.find(name);

    if (it != m_resourceMap.end()) {
        return it->second;
    }

    return nullptr;
}

void Technique::Update()
{
    //
}

void Technique::Render()
{
    uint32_t passIndex = 0;

    for (auto pass : m_Passes) {
        passIndex++;
        if (pass->IsEnabled()) {
            pass->PreRender();
            pass->Render(nullptr);
            pass->PostRender();
        }
    }
}

}    // namespace ade
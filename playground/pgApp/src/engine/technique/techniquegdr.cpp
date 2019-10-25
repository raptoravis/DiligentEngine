#include "TechniqueGdr.h"

#include "engine/mesh/meshcube.h"
#include "engine/mesh/meshcubetex.h"
#include "engine/pass/passgltfmodel.h"

#include "engine/pipeline/pipelinecolorvertex.h"
#include "engine/pipeline/pipelinetexvertex.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"
#include "engine/pass/passtest.h"

namespace ade
{

TechniqueGdr::TechniqueGdr(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    {}

    //
    {
        auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }
}


TechniqueGdr::~TechniqueGdr()
{
    //
}

void TechniqueGdr::Render()
{
    base::Render();
}

void TechniqueGdr::Update() {}

void TechniqueGdr::init()
{
    //
}

}    // namespace ade
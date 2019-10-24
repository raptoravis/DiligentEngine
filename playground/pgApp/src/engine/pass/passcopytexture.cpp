#include "passcopytexture.h"

namespace ade
{

PassCopyTexture::PassCopyTexture(Technique* parentTechnique,
                                 std::shared_ptr<Texture> dstTexture,
                                 std::shared_ptr<Texture> srcTexture)
    : base(parentTechnique), m_dstTexture(dstTexture), m_srcTexture(srcTexture)
{
}

PassCopyTexture::~PassCopyTexture() {}

void PassCopyTexture::Render(Pipeline* pipeline)
{
    Diligent::CopyTextureAttribs CopyAttribs(
        m_srcTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        m_dstTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    CopyAttribs.SrcMipLevel = 0;
    CopyAttribs.DstMipLevel = 0;
    CopyAttribs.DstSlice = 0;

    App::s_ctx->CopyTexture(CopyAttribs);
}
}    // namespace ade
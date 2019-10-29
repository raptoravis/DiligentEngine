
#include "gdr_common.sh"

Texture2D s_texOcclusionDepth : register( t0 );
RWTexture2D<float4> u_texOcclusionDepthOut : register( u0 );

[numthreads( 16, 16, 1 )]
void main(ComputeShaderInput IN)
{
	// this shader can be used to both copy a mip over to the output and downscale it.

	uint2 coord = uint2(IN.dispatchThreadID.xy);

	if (all(coord.xy < u_inputRTSize.xy))
	{
		float maxDepth = 1.0;

		float4 depths = float4(
			  s_texOcclusionDepth[uint2(u_inputRTSize.zw * coord.xy                  ) ].x
			, s_texOcclusionDepth[uint2(u_inputRTSize.zw * coord.xy + uint2(1.0, 0.0)) ].x
			, s_texOcclusionDepth[uint2(u_inputRTSize.zw * coord.xy + uint2(0.0, 1.0)) ].x
			, s_texOcclusionDepth[uint2(u_inputRTSize.zw * coord.xy + uint2(1.0, 1.0)) ].x
			);

		// find and return max depth
		maxDepth = max(
				max(depths.x, depths.y)
			, max(depths.z, depths.w)
			);

		u_texOcclusionDepthOut[coord] = float4(maxDepth,0,0,1);
	}
}

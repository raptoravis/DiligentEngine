
#include "gdr_common.sh"

Texture2D t_texOcclusionDepth : register( t0 );
RWTexture2D<float> u_texOcclusionDepthOut : register( u0 );

[numthreads( 16, 16, 1 )]
void main(ComputeShaderInput IN)
{
	// this shader can be used to both copy a mip over to the output and downscale it.

	int2 coord = int2(IN.dispatchThreadID.xy);

	if (all(coord.xy < u_inputRTSize.xy))
	{
		float maxDepth = 1.0;

		float4 depths = float4(
			  t_texOcclusionDepth[int2(u_inputRTSize.zw * coord.xy                  ) ].x
			, t_texOcclusionDepth[int2(u_inputRTSize.zw * coord.xy + int2(1.0, 0.0)) ].x
			, t_texOcclusionDepth[int2(u_inputRTSize.zw * coord.xy + int2(0.0, 1.0)) ].x
			, t_texOcclusionDepth[int2(u_inputRTSize.zw * coord.xy + int2(1.0, 1.0)) ].x
			);

		// find and return max depth
		maxDepth = max(
				max(depths.x, depths.y)
			, max(depths.z, depths.w)
			);

		u_texOcclusionDepthOut[coord] = maxDepth;
	}
}

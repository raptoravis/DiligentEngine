
#include "gdr_common.sh"

Texture2D s_texOcclusionDepth : register( t0 );

Buffer<float4>  instanceDataIn : register( t1 );

RWBuffer<uint> drawcallInstanceCount : register(u0);
RWBuffer<bool> instancePredicates : register(u1);

[numthreads( 64, 1, 1 )]
void main(ComputeShaderInput IN)
{
	bool predicate = false;

	//make sure that we not processing more instances than available
	if (IN.dispatchThreadID.x < uint(u_cullingConfig.x) )
	{
		//get the bounding box for this instance
		float4 bboxMin = instanceDataIn[2 * IN.dispatchThreadID.x] ;
		float3 bboxMax = instanceDataIn[2 * IN.dispatchThreadID.x + 1].xyz;

		int drawcallID = int(bboxMin.w);

		//Adapted from http://blog.selfshadow.com/publications/practical-visibility/
		float3 bboxSize = bboxMax.xyz - bboxMin.xyz;

		float3 boxCorners[] = {
			bboxMin.xyz,
			bboxMin.xyz + float3(bboxSize.x,0,0),
			bboxMin.xyz + float3(0, bboxSize.y,0),
			bboxMin.xyz + float3(0, 0, bboxSize.z),
			bboxMin.xyz + float3(bboxSize.xy,0),
			bboxMin.xyz + float3(0, bboxSize.yz),
			bboxMin.xyz + float3(bboxSize.x, 0, bboxSize.z),
			bboxMin.xyz + bboxSize.xyz
		};
		float minZ = 1.0;
		float2 minXY = float2(1.0, 1.0);
		float2 maxXY = float2(0.0, 0.0);

		for (int i = 0; i < 8; i++)
		{
			//transform World space aaBox to NDC
			float4 clipPos = mul( u_viewProj, float4(boxCorners[i], 1) );

#if BGFX_SHADER_LANGUAGE_GLSL 
			clipPos.z = 0.5 * ( clipPos.z + clipPos.w );
#endif
			clipPos.z = max(clipPos.z, 0);

			clipPos.xyz = clipPos.xyz / clipPos.w;

			clipPos.xy = clamp(clipPos.xy, -1, 1);
			clipPos.xy = clipPos.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

			minXY = min(clipPos.xy, minXY);
			maxXY = max(clipPos.xy, maxXY);

			minZ = saturate(min(minZ, clipPos.z));
		}

		float4 boxUVs = float4(minXY, maxXY);

		// Calculate hi-Z buffer mip
		float2 size = float2( (maxXY - minXY) * u_inputRTSize.xy);
		float mip = ceil(log2(max(size.x, size.y)));

		mip = clamp(mip, 0, u_cullingConfig.z);

		// Texel footprint for the lower (finer-grained) level
		float level_lower = max(mip - 1, 0);
		float2 scale = float2(exp2(-level_lower), exp2(-level_lower));
		float2 a = floor(boxUVs.xy*scale);
		float2 b = ceil(boxUVs.zw*scale);
		float2 dims = b - a;

		// Use the lower level if we only touch <= 2 texels in both dimensions
		if (dims.x <= 2 && dims.y <= 2)
			mip = level_lower;

#if BGFX_SHADER_LANGUAGE_GLSL
		boxUVs.y = 1.0 - boxUVs.y;
		boxUVs.w = 1.0 - boxUVs.w;
#endif
		//load depths from high z buffer
		float4 depth =
		{
			s_texOcclusionDepth.Load(uint3(boxUVs.xy, mip)).x,
			s_texOcclusionDepth.Load(uint3(boxUVs.zy, mip)).x,
			s_texOcclusionDepth.Load(uint3(boxUVs.xw, mip)).x,
			s_texOcclusionDepth.Load(uint3(boxUVs.zw, mip)).x,
		};

		//find the max depth
		float maxDepth = max( max(depth.x, depth.y), max(depth.z, depth.w) );

		if ( minZ <= maxDepth )
		{
			predicate = true;

			//increase instance count for this particular prop type
			uint value;
			InterlockedAdd(drawcallInstanceCount[ drawcallID ], 1, value);
		}
	}

	instancePredicates[IN.dispatchThreadID.x] = predicate;
}

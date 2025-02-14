
#include "gdr_common.sh"


float4 main(VertexShaderOutput IN) : SV_TARGET
{
	float4 color = u_color[uint(IN.materialID)];

	if (color.w < 1.0f)
	{
		//render dithered alpha
		if ( (int(IN.position.x) % 2) == (int(IN.position.y) % 2) )
		{
			discard;
		}
	}

	return float4(color.xyz, 1.0);
}
